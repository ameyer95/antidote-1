#ifndef BOXBOUNDEDDISJUNCTSDOMAINTEMPLATELABELS_HPP
#define BOXBOUNDEDDISJUNCTSDOMAINTEMPLATELABELS_HPP

#include "BoxDisjunctsDomainTemplateLabels.hpp"
#include "BoxStateDomainTemplateLabels.hpp"
#include "CommonEnums.h"
#include "Feature.hpp"
#include "StateDomainTemplate.hpp"
#include <forward_list>
#include <queue> // for priority_queue
#include <set>
#include <vector>


/**
 * The bounded disjuncts domain manipulates the same
 * box disjuncts as the unbounded domain,
 * but its transformers are responsible for maintaining the bound.
 * We use the structure of BoxDisjunctsDomain to assume
 * that the number of disjuncts changes only from applyFilter(Negated) and join
 */


// S is the "score" type (what joinPrecisionLoss returns); please use int, double, etc (needs < implemented)
template <typename T, typename P, typename D, typename S>
class BoxBoundedDisjunctsDomainTemplateLabels : public StateDomainTemplate<typename BoxDisjunctsTypesLabels<T,P,D>::Many> {
public:
    typedef BoxDisjunctsTypesLabels<T,P,D> Types;

private:
    // The main operation of combining disjuncts uses a priority queue to greedily merge pairs.
    // We need some auxiliary types for this.
    struct ScoreTuple {
        const typename Types::Single *e1;
        const typename Types::Single *e2;
        S score;
    };
    struct GreaterThanForScoreTuple { // The comparator for the priority queue
        bool operator()(const ScoreTuple &a, const ScoreTuple &b) const {
            // std::priority_queue prioritizes the maximal element by this ordering;
            // we would like b to be higher priority than a (i.e. this function returns true)
            // exactly when a's imprecision increase/score is greater than b's.
            return a.score > b.score;
        }
    };
    typedef std::priority_queue<ScoreTuple, std::vector<ScoreTuple>, GreaterThanForScoreTuple> ScoreQueue;

    typename Types::Many combined(const typename Types::Many &element) const;
    // Some subroutines for the above
    void initializeMerging(ScoreQueue &score_queue, std::set<const typename Types::Single *> &included, std::queue<const typename Types::Single *> &pending, const typename Types::Many &disjuncts) const;
    ScoreTuple selectMerge(ScoreQueue &score_queue, const std::set<const typename Types::Single *> &included) const;
    void performMerge(const ScoreTuple &to_merge, ScoreQueue &score_queue, std::set<const typename Types::Single *> &included, std::forward_list<typename Types::Single> &new_disjuncts) const;
    void prepareNextGreedyStep(ScoreQueue &score_queue, std::set<const typename Types::Single *> &included, std::queue<const typename Types::Single *> &pending) const;

    unsigned int max_num_disjuncts;
    DisjunctsMergeMode merge_mode;

public:
    const BoxDisjunctsDomainTemplateLabels<T,P,D> *disjuncts_domain;
    // We have access to the underlying box domain through disjuncts_domain->box_domain

    BoxBoundedDisjunctsDomainTemplateLabels(const BoxDisjunctsDomainTemplateLabels<T,P,D> *disjuncts_domain);
    BoxBoundedDisjunctsDomainTemplateLabels(const BoxDisjunctsDomainTemplateLabels<T,P,D> *disjuncts_domain, unsigned int max_num_disjuncts, const DisjunctsMergeMode &merge_mode);

    void setMergeDetails(unsigned int max_num_disjuncts, const DisjunctsMergeMode &merge_mode);

    virtual S joinPrecisionLoss(const typename Types::Single &e1, const typename Types::Single &e2) const = 0;

    typename Types::Many meetImpurityEqualsZero(const typename Types::Many &element) const;
    typename Types::Many meetImpurityNotEqualsZero(const typename Types::Many &element) const;
    typename Types::Many meetPhiIsBottom(const typename Types::Many &element) const;
    typename Types::Many meetPhiIsNotBottom(const typename Types::Many &element) const;
    typename Types::Many meetXModelsPhi(const typename Types::Many &element, const FeatureVector &x) const;
    typename Types::Many meetXNotModelsPhi(const typename Types::Many &element, const FeatureVector &x) const;
    typename Types::Many applyBestSplit(const typename Types::Many &element) const;
    typename Types::Many applySummary(const typename Types::Many &element) const;
    typename Types::Many applyFilter(const typename Types::Many &element) const;
    typename Types::Many applyFilterNegated(const typename Types::Many &element) const;

    bool isBottomElement(const typename Types::Many &element) const;
    typename Types::Many binary_join(const typename Types::Many &e1, const typename Types::Many &e2) const;
    typename Types::Many join(const std::vector<typename Types::Many> &elements) const;
};


/**
 * Queue-disjunct-merging methods
 */

// TODO could probably change it so that this mutates the disjunction...
template <typename T, typename P, typename D, typename S>
typename BoxDisjunctsTypesLabels<T,P,D>::Many BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::combined(const typename Types::Many &element) const {
    if(element.size() <= max_num_disjuncts) {
        return element;
    }

    ScoreQueue score_queue;
    // included stores pointers to disjuncts in elements, new_disjuncts, and pending.
    std::set<const typename Types::Single *> included;
    // We keep track of pointers to elements in the following container, so we can't use vector,
    // since when the vector is resized etc the memory locations of the objects can change.
    std::forward_list<typename Types::Single> new_disjuncts;
    // pending contains pointers to elements that are not yet in included.
    // Note that this is only non-empty for DisjunctsMergeMode::GREEDY,
    // since with DisjunctsMergeMode::OPTIMAL all of the disjuncts are initially included.
    std::queue<const typename Types::Single *> pending;

    // In general, in DisjunctsMergeMode::OPTIMAL we put all of the disjuncts into included and reduce one-by-one.
    // In DisjunctsMergeMode::GREEDY we put max_num_disjuncts+1 disjuncts into included, the rest into pending,
    // and then iteratively merge one disjunct in included + insert the next one from pending.
    initializeMerging(score_queue, included, pending, element);
    while(included.size() > max_num_disjuncts) {
        ScoreTuple to_merge = selectMerge(score_queue, included);
        performMerge(to_merge, score_queue, included, new_disjuncts);
        if(merge_mode == DisjunctsMergeMode::GREEDY) {
            prepareNextGreedyStep(score_queue, included, pending);
        }
    }

    // Finally, we construct the return object from the disjuncts to be included
    typename Types::Many ret;
    for(auto i = included.begin(); i != included.end(); i++) {
        ret.push_back(**i);
    }
    return ret;
}

template <typename T, typename P, typename D, typename S>
void BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::initializeMerging(ScoreQueue &score_queue, std::set<const typename Types::Single *> &included, std::queue<const typename Types::Single *> &pending, const typename Types::Many &disjuncts) const {
    if(merge_mode == DisjunctsMergeMode::OPTIMAL) {
        // Initially, included has a pointer to each of the original disjuncts
        for(auto i = disjuncts.cbegin(); i != disjuncts.cend(); i++) {
            included.insert(&*i);
        }
    } else { // i.e. merge_mode == DisjunctsMergeMode::GREEDY
        // Initially, included has a pointer to only the first max_num_disjuncts+1 elements
        auto disjuncts_iter = disjuncts.begin();
        for(unsigned int i = 0; i < max_num_disjuncts + 1; i++, disjuncts_iter++) {
            // Because this whole function is called in the else-branch of a check that disjuncts.size() <= max_num_disjuncts,
            // we can loop based on numeric values without worrying about the vector iterator being out-of-bounds.
            included.insert(&*disjuncts_iter);
        }
        // And the remainder are stored in pending
        for(/*from where disjuncts_iter left off*/; disjuncts_iter != disjuncts.end(); disjuncts_iter++) {
            pending.push(&*disjuncts_iter);
        }
    }
    // In either case, scores for each pair of elements in included are added to the score queue
    for(auto i = included.cbegin(); i != included.cend(); i++) {
        for(auto j = i; j != included.cend(); j++) {
            if(i == j) {
                continue;
            }
            ScoreTuple temp = {*i, *j, joinPrecisionLoss(**i, **j)};
            score_queue.push(temp);
        }
    }
}

template <typename T, typename P, typename D, typename S>
typename BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::ScoreTuple BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::selectMerge(ScoreQueue &score_queue, const std::set<const typename Types::Single *> &included) const {
    ScoreTuple ret;
    do {
        ret = score_queue.top();
        score_queue.pop();
    } while(included.find(ret.e1) == included.end() || included.find(ret.e2) == included.end());
    // XXX it should be an invariant that this loop always terminates in a correct fashion, but could be cleaner
    return ret;
}

template <typename T, typename P, typename D, typename S>
void BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::performMerge(const ScoreTuple &to_merge, ScoreQueue &score_queue, std::set<const typename Types::Single *> &included, std::forward_list<typename Types::Single> &new_disjuncts) const {
    included.erase(to_merge.e1);
    included.erase(to_merge.e2);
    new_disjuncts.push_front(disjuncts_domain->box_domain->binary_join(*to_merge.e1, *to_merge.e2));
    // We first compute the relevant scores to add to the priority queue
    // before adding the new disjunct to included.
    for(auto i = included.begin(); i != included.end(); i++) {
        ScoreTuple temp = {*i, &new_disjuncts.front(), joinPrecisionLoss(**i, new_disjuncts.front())};
        score_queue.push(temp);
    }
    included.insert(&new_disjuncts.front());
}

template <typename T, typename P, typename D, typename S>
void BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::prepareNextGreedyStep(ScoreQueue &score_queue, std::set<const typename Types::Single *> &included, std::queue<const typename Types::Single *> &pending) const {
    // For DisjunctsMergeMode::GREEDY, we have to move something from pending to included and add relevant scores to the ScoreQueue.
    // Note that in DisjunctsMergeMode::OPTIMAL pending is always empty.
    if(pending.size() > 0) {
        for(auto i = included.begin(); i != included.end(); i++) {
            ScoreTuple temp = {*i, pending.front(), joinPrecisionLoss(**i, *pending.front())};
            score_queue.push(temp);
        }
        included.insert(pending.front());
        pending.pop();
    }
}

/**
 * Constructors and public setter
 */

template <typename T, typename P, typename D, typename S>
inline BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::BoxBoundedDisjunctsDomainTemplateLabels(const BoxDisjunctsDomainTemplateLabels<T,P,D> *disjuncts_domain) {
    this->disjuncts_domain = disjuncts_domain;
    this->max_num_disjuncts = 1;
    this->merge_mode = DisjunctsMergeMode::OPTIMAL;
}

template <typename T, typename P, typename D, typename S>
inline BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::BoxBoundedDisjunctsDomainTemplateLabels(const BoxDisjunctsDomainTemplateLabels<T,P,D> *disjuncts_domain, unsigned int max_num_disjuncts, const DisjunctsMergeMode &merge_mode) {
    this->disjuncts_domain = disjuncts_domain;
    this->max_num_disjuncts = max_num_disjuncts;
    this->merge_mode = merge_mode;
}

template <typename T, typename P, typename D, typename S>
inline void BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::setMergeDetails(unsigned int max_num_disjuncts, const DisjunctsMergeMode &merge_mode) {
    this->max_num_disjuncts = max_num_disjuncts;
    this->merge_mode = merge_mode;
}

/**
 * StateDomainTemplate virtual method implementations
 */

template <typename T, typename P, typename D, typename S>
inline typename BoxDisjunctsTypesLabels<T,P,D>::Many BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::meetImpurityEqualsZero(const typename Types::Many &element) const {
    return disjuncts_domain->meetImpurityEqualsZero(element);
}

template <typename T, typename P, typename D, typename S>
inline typename BoxDisjunctsTypesLabels<T,P,D>::Many BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::meetImpurityNotEqualsZero(const typename Types::Many &element) const {
    return disjuncts_domain->meetImpurityNotEqualsZero(element);
}

template <typename T, typename P, typename D, typename S>
inline typename BoxDisjunctsTypesLabels<T,P,D>::Many BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::meetPhiIsBottom(const typename Types::Many &element) const {
    return disjuncts_domain->meetPhiIsBottom(element);
}

template <typename T, typename P, typename D, typename S>
inline typename BoxDisjunctsTypesLabels<T,P,D>::Many BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::meetPhiIsNotBottom(const typename Types::Many &element) const {
    return disjuncts_domain->meetPhiIsNotBottom(element);
}

template <typename T, typename P, typename D, typename S>
inline typename BoxDisjunctsTypesLabels<T,P,D>::Many BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::meetXModelsPhi(const typename Types::Many &element, const FeatureVector &x) const {
    return disjuncts_domain->meetXModelsPhi(element, x);
}

template <typename T, typename P, typename D, typename S>
inline typename BoxDisjunctsTypesLabels<T,P,D>::Many BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::meetXNotModelsPhi(const typename Types::Many &element, const FeatureVector &x) const {
    return disjuncts_domain->meetXNotModelsPhi(element, x);
}

template <typename T, typename P, typename D, typename S>
inline typename BoxDisjunctsTypesLabels<T,P,D>::Many BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::applyBestSplit(const typename Types::Many &element) const {
    return disjuncts_domain->applyBestSplit(element);
}

template <typename T, typename P, typename D, typename S>
inline typename BoxDisjunctsTypesLabels<T,P,D>::Many BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::applySummary(const typename Types::Many &element) const {
    return disjuncts_domain->applySummary(element);
}

template <typename T, typename P, typename D, typename S>
inline typename BoxDisjunctsTypesLabels<T,P,D>::Many BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::applyFilter(const typename Types::Many &element) const {
    return combined(disjuncts_domain->applyFilter(element));
}

template <typename T, typename P, typename D, typename S>
inline typename BoxDisjunctsTypesLabels<T,P,D>::Many BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::applyFilterNegated(const typename Types::Many &element) const {
    return combined(disjuncts_domain->applyFilterNegated(element));
}

/**
 * Joins etc
 */

template <typename T, typename P, typename D, typename S>
inline bool BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::isBottomElement(const typename Types::Many &element) const {
    return disjuncts_domain->isBottomElement(element);
}

template <typename T, typename P, typename D, typename S>
inline typename BoxDisjunctsTypesLabels<T,P,D>::Many BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::binary_join(const typename Types::Many &e1, const typename Types::Many &e2) const {
    return combined(disjuncts_domain->binary_join(e1, e2));
}

template <typename T, typename P, typename D, typename S>
inline typename BoxDisjunctsTypesLabels<T,P,D>::Many BoxBoundedDisjunctsDomainTemplateLabels<T,P,D,S>::join(const std::vector<typename Types::Many> &elements) const {
    return combined(disjuncts_domain->join(elements));
}

#endif
