#ifndef BOXBOUNDEDDISJUNCTSDOMAIN_H
#define BOXBOUNDEDDISJUNCTSDOMAIN_H

#include "StateDomain.h"
#include "BoxDomain.h"
#include <forward_list>
#include <queue> // for priority_queue
#include <set>
#include <utility>
#include <vector>


/**
 * The bounded disjuncts domain manipulates the same
 * box disjuncts as the unbounded domain,
 * but its transformers are responsible for maintaining the bound.
 * We use the structure of BoxDisjunctsDomain to assume
 * that the number of disjuncts changes only from applyFilter(Negated) and join
 */

// LA should be the underlying BoxDisjunctsDomain<A> class
// A should be the BoxDisjunctsStateAbstraction<B> class
// B should be the underlying BoxStateAbstraction<T,P,D> class
// S is the "score" type (what joinPrecisionLoss returns); please use int, double, etc (needs < implemented)
template <typename LA, typename A, typename B, typename S>
class BoxBoundedDisjunctsDomain : public StateDomain<A> {
public: enum class MergeMode { GREEDY, OPTIMAL };

private:
    // The main operation of combining disjuncts uses a priority queue to greedily merge pairs.
    // We need some auxiliary types for this.
    struct ScoreTuple {
        const B *e1;
        const B *e2;
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

    A combined(const A &element) const;
    // Some subroutines for the above
    inline ScoreTuple selectMerge(ScoreQueue &score_queue, const std::set<const B *> &included) const;
    inline void initializeMerging(ScoreQueue &score_queue, std::set<const B *> &included, std::queue<const B *> &pending, const std::vector<B> &disjuncts) const;
    inline void performMerge(const ScoreTuple &to_merge, ScoreQueue &score_queue, std::set<const B *> &included, std::forward_list<B> &new_disjuncts) const;
    inline void prepareNextGreedyStep(ScoreQueue &score_queue, std::set<const B *> &included, std::queue<const B *> &pending) const;

protected:
    const LA *disjuncts_domain; // XXX subclass needs to populate this
    unsigned int max_num_disjuncts; // XXX subclass populates this
    // We have access to the underlying box domain (const LB *) through disjuncts_domain->box_domain
    // (note LB is not actually a template parameter here; sorry if this is confusing).
    MergeMode merge_mode; // XXX subclass populates this

public:
    virtual S joinPrecisionLoss(const B &e1, const B &e2) const = 0;

    A meetImpurityEqualsZero(const A &element) const { return disjuncts_domain->meetImpurityEqualsZero(element); }
    A meetImpurityNotEqualsZero(const A &element) const { return disjuncts_domain->meetImpurityNotEqualsZero(element); }
    A meetPhiIsBottom(const A &element) const { return disjuncts_domain->meetPhiIsBottom(element); }
    A meetPhiIsNotBottom(const A &element) const { return disjuncts_domain->meetPhiIsNotBottom(element); }
    A meetXModelsPhi(const A &element) const { return disjuncts_domain->meetXModelsPhi(element); }
    A meetXNotModelsPhi(const A &element) const { return disjuncts_domain->meetXNotModelsPhi(element); }
    A applyBestSplit(const A &element) const { return disjuncts_domain->applyBestSplit(element); }
    A applySummary(const A &element) const { return disjuncts_domain->applySummary(element); }
    A applyFilter(const A &element) const { return combined(disjuncts_domain->applyFilter(element)); }
    A applyFilterNegated(const A &element) const { return combined(disjuncts_domain->applyFilterNegated(element)); }

    A binary_join(const A &e1, const A &e2) const { return combined(disjuncts_domain->binary_join(e1, e2)); }
    A join(const std::vector<A> &elements) const { return combined(disjuncts_domain->join(elements)); }
};


// TODO could probably change it so that this mutates the disjunction...
template <typename LA, typename A, typename B, typename S>
A BoxBoundedDisjunctsDomain<LA,A,B,S>::combined(const A &element) const {
    if(element.disjuncts.size() <= max_num_disjuncts) {
        return element;
    }

    ScoreQueue score_queue;
    std::set<const B *> included; // Pointers to elements.disjuncts, new_disjuncts, and pending
    std::forward_list<B> new_disjuncts; // We keep track of pointers to elements in this container, so we can't use vector,
                                        // since when the vector is resized etc the memory locations of the objects can change.
    std::queue<const B *> pending; // Pointers to elements.disjuncts that have not yet made their way into included.
                                   // Note that this is only non-empty for MergeMode::GREEDY,
                                   // since with MergeMode::OPTIMAL all of the disjuncts are initially included.

    // In general, in MergeMode::OPTIMAL we put all of the disjuncts into included and reduce one-by-one.
    // In MergeMode::GREEDY we put max_num_disjuncts+1 disjuncts into included, the rest into pending,
    // and then iteratively merge one disjunct in included + insert the next one from pending.
    initializeMerging(score_queue, included, pending, element.disjuncts);
    while(included.size() > max_num_disjuncts) {
        ScoreTuple to_merge = selectMerge(score_queue, included);
        performMerge(to_merge, score_queue, included, new_disjuncts);
        if(merge_mode == MergeMode::GREEDY) {
            prepareNextGreedyStep(score_queue, included, pending);
        }
    }

    // Finally, we construct the return object from the disjuncts to be included
    std::vector<B> ret;
    for(typename std::set<const B *>::const_iterator i = included.begin(); i != included.end(); i++) {
        ret.push_back(**i);
    }
    return A(ret);
}

template <typename LA, typename A, typename B, typename S>
void BoxBoundedDisjunctsDomain<LA,A,B,S>::initializeMerging(ScoreQueue &score_queue, std::set<const B *> &included, std::queue<const B *> &pending, const std::vector<B> &disjuncts) const {
    if(merge_mode == MergeMode::OPTIMAL) {
        // Initially, included has a pointer to each of the original disjuncts
        for(typename std::vector<B>::const_iterator i = disjuncts.begin(); i != disjuncts.end(); i++) {
            included.insert(&*i);
        }
    } else { // i.e. merge_mode == MergeMode::GREEDY
        // Initially, included has a pointer to only the first max_num_disjuncts+1 elements
        typename std::vector<B>::const_iterator disjuncts_iter = disjuncts.begin();
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
    for(typename std::set<const B *>::const_iterator i = included.begin(); i != included.end(); i++) {
        for(typename std::set<const B *>::const_iterator j = i; j != included.end(); j++) {
            if(i == j) {
                continue;
            }
            ScoreTuple temp = {*i, *j, joinPrecisionLoss(**i, **j)};
            score_queue.push(temp);
        }
    }
}

template <typename LA, typename A, typename B, typename S>
typename BoxBoundedDisjunctsDomain<LA,A,B,S>::ScoreTuple BoxBoundedDisjunctsDomain<LA,A,B,S>::selectMerge(ScoreQueue &score_queue, const std::set<const B *> &included) const {
    ScoreTuple ret;
    do {
        ret = score_queue.top();
        score_queue.pop();
    } while(included.find(ret.e1) == included.end() || included.find(ret.e2) == included.end());
    // XXX it should be an invariant that this loop always terminates in a correct fashion, but could be cleaner
    return ret;
}

template <typename LA, typename A, typename B, typename S>
void BoxBoundedDisjunctsDomain<LA,A,B,S>::performMerge(const ScoreTuple &to_merge, ScoreQueue &score_queue, std::set<const B *> &included, std::forward_list<B> &new_disjuncts) const {
    included.erase(to_merge.e1);
    included.erase(to_merge.e2);
    new_disjuncts.push_front(disjuncts_domain->box_domain->binary_join(*to_merge.e1, *to_merge.e2));
    // We first compute the relevant scores to add to the priority queue
    // before adding the new disjunct to included.
    for(typename std::set<const B *>::const_iterator i = included.begin(); i != included.end(); i++) {
        ScoreTuple temp = {*i, &new_disjuncts.front(), joinPrecisionLoss(**i, new_disjuncts.front())};
        score_queue.push(temp);
    }
    included.insert(&new_disjuncts.front());
}

template <typename LA, typename A, typename B, typename S>
void BoxBoundedDisjunctsDomain<LA,A,B,S>::prepareNextGreedyStep(ScoreQueue &score_queue, std::set<const B *> &included, std::queue<const B *> &pending) const {
    // For MergeMode::GREEDY, we have to move something from pending to included and add relevant scores to the ScoreQueue.
    // Note that in MergeMode::OPTIMAL pending is always empty.
    if(pending.size() > 0) {
        for(typename std::set<const B *>::const_iterator i = included.begin(); i != included.end(); i++) {
            ScoreTuple temp = {*i, pending.front(), joinPrecisionLoss(**i, *pending.front())};
            score_queue.push(temp);
        }
        included.insert(pending.front());
        pending.pop();
    }
}

#endif
