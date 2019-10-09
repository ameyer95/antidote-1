#ifndef BOXBOUNDEDDISJUNCTSDOMAIN_H
#define BOXBOUNDEDDISJUNCTSDOMAIN_H

#include "StateDomain.h"
#include "BoxDomain.h"
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
private:
    // The main operation of combining disjuncts uses a priority queue to greedily merge pairs.
    // We need some auxiliary types for this.
    struct ScoreTuple {
        const B *e1;
        const B *e2;
        S score;
    };
    struct LessThanForScoreTuple { // The comparator for the priority queue
        bool operator()(const ScoreTuple &a, const ScoreTuple &b) const {
            // Returns true when a is higher piority than b;
            // in our case, this is when a's imprecision increase/score is less than b's.
            return a.score < b.score;
        }
    };
    typedef std::priority_queue<ScoreTuple, std::vector<ScoreTuple>, LessThanForScoreTuple> ScoreQueue;

    inline void populateInitialScores(const std::set<const B *> &included, ScoreQueue &score_queue) const;
    inline ScoreTuple greedySelection(const std::set<const B *> &included, ScoreQueue &score_queue) const;

protected:
    const LA *disjuncts_domain; // XXX subclass needs to populate this
    unsigned int max_num_disjuncts; // TODO populate this
    // We have access to the underlying box domain through LA

public:
    virtual S joinPrecisionLoss(const B &e1, const B &e2) const = 0;
    A combined(const A &element) const;

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
    std::vector<B> new_disjuncts;
    std::set<const B *> included; // Pointers to elements.disjuncts and new_disjuncts
    // Initially, included has a pointer to each disjuncts in element.disjuncts
    for(typename std::vector<B>::const_iterator i = element.disjuncts.begin(); i != element.disjuncts.end(); i++) {
        included.insert(&*i);
    }
    populateInitialScores(included, score_queue);

    // We gradually remove pairs of disjuncts from included (using score_queue to pick),
    // put the join of those disjuncts into new_disjuncts,
    // and insert a pointer to that new disjunct into included.
    while(included.size() > max_num_disjuncts) {
        ScoreTuple to_merge = greedySelection(included, score_queue);
        included.erase(to_merge.e1);
        included.erase(to_merge.e2);
        new_disjuncts.push_back(disjuncts_domain->box_domain->binary_join(*to_merge.e1, *to_merge.e2));
        // We first compute the relevant scores to add the priority queue
        // before adding the new disjunct to included
        for(typename std::set<const B *>::const_iterator i = included.begin(); i != included.end(); i++) {
            ScoreTuple temp = {*i, &new_disjuncts.back(), joinPrecisionLoss(**i, new_disjuncts.back())};
            score_queue.push(temp);
        }
        included.insert(&new_disjuncts.back());
    }

    // Finally, we construct the return object from the disjuncts to be included
    std::vector<B> ret;
    for(typename std::set<const B *>::const_iterator i = included.begin(); i != included.end(); i++) {
        ret.push_back(**i);
    }
    return A(ret);
}

template <typename LA, typename A, typename B, typename S>
void BoxBoundedDisjunctsDomain<LA,A,B,S>::populateInitialScores(const std::set<const B *> &included, BoxBoundedDisjunctsDomain<LA,A,B,S>::ScoreQueue &score_queue) const {
    for(typename std::set<const B *>::const_iterator i = included.begin(); i != included.end(); i++) {
        for(typename std::set<const B *>::const_iterator j = i; j != included.end(); j++) {
            if(i == j) {
                continue;
            }
            BoxBoundedDisjunctsDomain<LA,A,B,S>::ScoreTuple temp = {*i, *j, joinPrecisionLoss(**i, **j)};
            score_queue.push(temp);
        }
    }
}

template <typename LA, typename A, typename B, typename S>
typename BoxBoundedDisjunctsDomain<LA,A,B,S>::ScoreTuple BoxBoundedDisjunctsDomain<LA,A,B,S>::greedySelection(const std::set<const B *> &included, BoxBoundedDisjunctsDomain<LA,A,B,S>::ScoreQueue &score_queue) const {
    ScoreTuple ret;
    do {
        ret = score_queue.top();
        score_queue.pop();
    } while(included.find(ret.e1) == included.end() || included.find(ret.e2) == included.end());
    // XXX it should be an invariant that this loop always terminates in a correct fashion, but could be cleaner
    return ret;
}

#endif
