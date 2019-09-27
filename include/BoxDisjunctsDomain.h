#ifndef BOXDISJUNCTSDOMAIN_H
#define BOXDISJUNCTSDOMAIN_H

#include "AbstractDomain.h"
#include "StateDomain.h"
#include "BoxDomain.h"
#include <vector>
#include <utility>


// B should be the individual BoxStateAbstraction<...> type
template <typename B>
class BoxDisjunctsStateAbstraction : public AbstractElement {
private:
    bool is_bottom_element_flag; // XXX this is not handled at all, basically
public:
    BoxDisjunctsStateAbstraction(const std::vector<B> &disjuncts) { this->disjuncts = disjuncts; }
    std::vector<B> disjuncts;
    bool isBottomElement() const { return is_bottom_element_flag; }
};


// A should be the BoxDisjunctsStateAbstraction<B> class
// LB should be the underlying BoxDomain<B> class
// B should be the underlying BoxStateAbstraction<T,P,D> class
// (but note the distribution abstraction template D is not needed here)
template <typename A, typename LB, typename B, typename T, typename P>
class BoxDisjunctsDomain : public StateDomain<A> {
private:
    LB box_domain;

    // Since most transformers just apply the underlying box domain's transformers
    // to each disjunct, we have an auxiliary method for that
    // that takes an appropriate function pointer as the second argument.
    inline A transformEachDisjunct(const A &element, B (LB::*fptr)(const B&) const) const;
    // The filter cases are slightly different
    inline A filterAndUnion(const A &element, bool negated) const;
    
public:
    virtual std::vector<std::pair<T, P>> filter(const T &training_set_abstraction, const P &predicate_abstraction) const = 0;
    virtual std::vector<std::pair<T, P>> filterNegated(const T &training_set_abstraction, const P &predicate_abstraction) const = 0;

    A meetImpurityEqualsZero(const A &element) const { return transformEachDisjunct(element, &LB::meetImpurityEqualsZero); }
    A meetImpurityNotEqualsZero(const A &element) const { return transformEachDisjunct(element, &LB::meetImpurityNotEqualsZero); }
    A meetPhiIsBottom(const A &element) const { return transformEachDisjunct(element, &LB::meetPhiIsBottom); }
    A meetPhiIsNotBottom(const A &element) const { return transformEachDisjunct(element, &LB::meetPhiIsNotBottom); }
    A meetXModelsPhi(const A &element) const { return transformEachDisjunct(element, &LB::meetXModelsPhi); }
    A meetXNotModelsPhi(const A &element) const { return transformEachDisjunct(element, &LB::meetXNotModelsPhi); }
    A applyBestSplit(const A &element) const { return transformEachDisjunct(element, &LB::applyBestSplit); }
    A applySummary(const A &element) const { return transformEachDisjunct(element, &LB::applySummary); }
    A applyFilter(const A &element) const { return filterAndUnion(element, false); }
    A applyFilterNegated(const A &element) const { return filterAndUnion(element, true); }

    A binary_join(const A &e1, const A &e2) const;
    A join(const std::vector<A> &elements) const; // We can override the aggregate to be more efficient
};


/**
 * BoxDisjunctsDomain member functions.
 */

template <typename A, typename LB, typename B, typename T, typename P>
inline A BoxDisjunctsDomain<A,LB,B,T,P>::transformEachDisjunct(const A &element, B (LB::*fptr)(const B&) const) const {
    std::vector<B> ret;
    for(typename std::vector<B>::const_iterator i = element.disjuncts.begin(); i != element.disjuncts.end(); i++) {
        B temp = (box_domain.*fptr)(*i);
        if(!temp.isBottomElement()) {
            ret.push_back(temp);
        }
    }
    return A(ret);
}

template <typename A, typename LB, typename B, typename T, typename P>
A BoxDisjunctsDomain<A,LB,B,T,P>::filterAndUnion(const A &element, bool negated) const {
    std::vector<B> ret;
    // For each disjunct
    for(typename std::vector<B>::const_iterator i = element.disjuncts.begin(); i != element.disjuncts.end(); i++) {
        // We get some number of <T,P> disjuncts back
        std::vector<std::pair<T, P>> temp;
        if(!negated) {
            temp = filter(i->training_set_abstraction, i->predicate_abstraction);
        } else {
            temp = filterNegated(i->training_set_abstraction, i->predicate_abstraction);
        }
        // Put all of them (with the appropriate posterior distribution abstract added) into the to-be-returned
        for(typename std::vector<std::pair<T, P>>::const_iterator j = temp.begin(); j != temp.end(); j++) {
            B temp_box(j->first, j->second, i->posterior_distribution_abstraction);
            if(!temp_box.isBottomElement()) {
                ret.push_back(temp_box);
            }
        }
    }
    return A(ret);
}

template <typename A, typename LB, typename B, typename T, typename P>
A BoxDisjunctsDomain<A,LB,B,T,P>::binary_join(const A &e1, const A &e2) const {
    std::vector<B> ret = e1.disjuncts;
    for(typename std::vector<B>::const_iterator i = e2.disjuncts.begin(); i != e2.disjuncts.end(); i++) {
        ret.push_beck(*i);
    }
    return A(ret);
}

template <typename A, typename LB, typename B, typename T, typename P>
A BoxDisjunctsDomain<A,LB,B,T,P>::join(const std::vector<A> &elements) const {
    std::vector<B> ret;
    for(typename std::vector<A>::const_iterator i = elements.begin(); i != elements.end(); i++) {
        for(typename std::vector<B>::const_iterator j = i->disjuncts.begin(); j != i->disjuncts.end(); j++) {
            ret.push_back(*j);
        }
    }
    return A(ret);
}

#endif
