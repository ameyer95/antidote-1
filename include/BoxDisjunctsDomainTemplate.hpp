#ifndef BOXDISJUNCTSDOMAINTEMPLATE_HPP
#define BOXDISJUNCTSDOMAINTEMPLATE_HPP

#include "BoxStateDomainTemplate.hpp"
#include "Feature.hpp"
#include "StateDomainTemplate.hpp"
#include <utility>
#include <vector>
#include <iostream>

// We'll output some information about the number of disjuncts
// when -DDEBUG is given as a g++ argument
#ifdef DEBUG
#include <iostream>
#endif

template <typename T, typename P, typename D>
struct BoxDisjunctsTypes {
    typedef BoxStateDomainTemplate<T,P,D> SingleDomain;
    typedef BoxStateAbstraction<T,P,D> Single;
    typedef std::vector<Single> Many;
};


template <typename T, typename P, typename D>
class BoxDisjunctsDomainTemplate : public StateDomainTemplate<typename BoxDisjunctsTypes<T,P,D>::Many> {
public: typedef BoxDisjunctsTypes<T,P,D> Types;

private:
    // Since most transformers just apply the underlying box domain's transformers
    // to each disjunct, we have an auxiliary method for that
    // that takes an appropriate function pointer as the second argument.
    typename Types::Many transformEachDisjunct(const typename Types::Many &element, typename Types::Single (Types::SingleDomain::*fptr)(const typename Types::Single&) const) const;
    // We repeat this for the functions that also take the test input as an argument
    // (i.e. have a different fptr signature)
    typename Types::Many transformEachDisjunct(const typename Types::Many &element, typename Types::Single (Types::SingleDomain::*fptr)(const typename Types::Single&, const FeatureVector&) const, const FeatureVector &x) const;
    // The filter cases are slightly different
    typename Types::Many filterAndUnion(const typename Types::Many &element, bool negated) const;

public:
    const typename Types::SingleDomain *box_domain;

    BoxDisjunctsDomainTemplate(const typename Types::SingleDomain *box_domain) { this->box_domain = box_domain; }

    virtual std::vector<std::pair<T, P>> filter(const T &training_set_abstraction, const P &predicate_abstraction) const = 0;
    virtual std::vector<std::pair<T, P>> filterNegated(const T &training_set_abstraction, const P &predicate_abstraction) const = 0;

    // TODO it is easy to imagine cases where the meetImpurity(Not)EqualsZero computations
    // should be able to split into individual disjuncts as well
    typename Types::Many meetImpurityEqualsZero(const typename Types::Many &element) const { return transformEachDisjunct(element, &Types::SingleDomain::meetImpurityEqualsZero); }
    typename Types::Many meetImpurityNotEqualsZero(const typename Types::Many &element) const { return transformEachDisjunct(element, &Types::SingleDomain::meetImpurityNotEqualsZero); }
    typename Types::Many meetPhiIsBottom(const typename Types::Many &element) const { return transformEachDisjunct(element, &Types::SingleDomain::meetPhiIsBottom); }
    typename Types::Many meetPhiIsNotBottom(const typename Types::Many &element) const { return transformEachDisjunct(element, &Types::SingleDomain::meetPhiIsNotBottom); }
    typename Types::Many meetXModelsPhi(const typename Types::Many &element, const FeatureVector &x) const { return transformEachDisjunct(element, &Types::SingleDomain::meetXModelsPhi, x); }
    typename Types::Many meetXNotModelsPhi(const typename Types::Many &element, const FeatureVector &x) const { return transformEachDisjunct(element, &Types::SingleDomain::meetXNotModelsPhi, x); }
    typename Types::Many applyBestSplit(const typename Types::Many &element) const { return transformEachDisjunct(element, &Types::SingleDomain::applyBestSplit); }
    typename Types::Many applySummary(const typename Types::Many &element) const { return transformEachDisjunct(element, &Types::SingleDomain::applySummary); }
    typename Types::Many applyFilter(const typename Types::Many &element) const { return filterAndUnion(element, false); }
    typename Types::Many applyFilterNegated(const typename Types::Many &element) const { return filterAndUnion(element, true); }


    bool isBottomElement(const typename Types::Many &element) const;
    typename Types::Many binary_join(const typename Types::Many &e1, const typename Types::Many &e2) const;
    typename Types::Many join(const std::vector<typename Types::Many> &elements) const; // We can override the aggregate to be more efficient
};


/**
 * BoxDisjunctsDomain member functions.
 */

template <typename T, typename P, typename D>
inline typename BoxDisjunctsTypes<T,P,D>::Many BoxDisjunctsDomainTemplate<T,P,D>::transformEachDisjunct(const typename Types::Many &element, typename Types::Single (Types::SingleDomain::*fptr)(const typename Types::Single&) const) const {
    typename Types::Many ret;
    for(auto i = element.cbegin(); i != element.cend(); i++) {
        typename Types::Single temp = (box_domain->*fptr)(*i);
        if(!box_domain->isBottomElement(temp)) {
            ret.push_back(temp);
        }
    }
    return ret;
}

template <typename T, typename P, typename D>
inline typename BoxDisjunctsTypes<T,P,D>::Many BoxDisjunctsDomainTemplate<T,P,D>::transformEachDisjunct(const typename Types::Many &element, typename Types::Single (Types::SingleDomain::*fptr)(const typename Types::Single&, const FeatureVector&) const, const FeatureVector &x) const {
    typename Types::Many ret;
    for(auto i = element.cbegin(); i != element.cend(); i++) {
        typename Types::Single temp = (box_domain->*fptr)(*i, x);
        if(!box_domain->isBottomElement(temp)) {
            ret.push_back(temp);
        }
    }
    return ret;
}

template <typename T, typename P, typename D>
typename BoxDisjunctsTypes<T,P,D>::Many BoxDisjunctsDomainTemplate<T,P,D>::filterAndUnion(const typename Types::Many &element, bool negated) const {
    typename Types::Many ret;
    // For each disjunct
    for(auto i = element.cbegin(); i != element.cend(); i++) {
        // We get some number of <T,P> disjuncts back
        std::vector<std::pair<T, P>> temp;
        if(!negated) {
            temp = filter(i->training_set_abstraction, i->predicate_abstraction);
        } else {
            temp = filterNegated(i->training_set_abstraction, i->predicate_abstraction);
        }
        // Put all of them (with the appropriate posterior distribution abstract added) into the to-be-returned
        for(auto j = temp.cbegin(); j != temp.cend(); j++) {
            typename Types::Single temp_box = {j->first, j->second, i->posterior_distribution_abstraction};
            if(!box_domain->isBottomElement(temp_box)) {
                ret.push_back(temp_box);
            }
        }
    }

#ifdef DEBUG
    std::cout << "exiting filterAndUnion with " << ret.size() << " disjuncts" << std::endl;
#endif
    return ret;
}

template <typename T, typename P, typename D>
bool BoxDisjunctsDomainTemplate<T,P,D>::isBottomElement(const typename Types::Many &element) const {
    return element.size() == 0; // XXX could/should check that each disjunct is non-bot TO DO ANNA?
}

template <typename T, typename P, typename D>
typename BoxDisjunctsTypes<T,P,D>::Many BoxDisjunctsDomainTemplate<T,P,D>::binary_join(const typename Types::Many &e1, const typename Types::Many &e2) const {
    typename Types::Many ret = e1;
    for(auto i = e2.cbegin(); i != e2.cend(); i++) {
        ret.push_back(*i);
    }
    return ret;
}

template <typename T, typename P, typename D>
typename BoxDisjunctsTypes<T,P,D>::Many BoxDisjunctsDomainTemplate<T,P,D>::join(const std::vector<typename Types::Many> &elements) const {
    typename Types::Many ret;
    for(auto i = elements.cbegin(); i != elements.cend(); i++) {
        for(auto j = i->cbegin(); j != i->cend(); j++) {
            ret.push_back(*j);
        }
    }
    return ret;
}

#endif
