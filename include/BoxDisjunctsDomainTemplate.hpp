#ifndef BOXDISJUNCTSDOMAINTEMPLATE_HPP
#define BOXDISJUNCTSDOMAINTEMPLATE_HPP

#include "BoxStateDomainTemplate.hpp"
#include "Feature.hpp"
#include "StateDomainTemplate.hpp"
#include <utility>
#include <vector>


template <typename T, typename P, typename D>
class BoxDisjunctsDomainTemplate : public StateDomainTemplate<std::vector<BoxStateAbstraction<T,P,D>>> {
public:
    typedef BoxStateAbstraction<T,P,D> SingleBoxType;
    typedef std::vector<SingleBoxType> BoxDisjunctsType;

private:
    // Since most transformers just apply the underlying box domain's transformers
    // to each disjunct, we have an auxiliary method for that
    // that takes an appropriate function pointer as the second argument.
    BoxDisjunctsType transformEachDisjunct(const BoxDisjunctsType &element, SingleBoxType (BoxStateDomainTemplate<T,P,D>::*fptr)(const SingleBoxType&) const) const;
    BoxDisjunctsType transformEachDisjunct(const BoxDisjunctsType &element, SingleBoxType (BoxStateDomainTemplate<T,P,D>::*fptr)(const SingleBoxType&, const FeatureVector&) const, const FeatureVector &x) const;
    // The filter cases are slightly different
    BoxDisjunctsType filterAndUnion(const BoxDisjunctsType &element, bool negated) const;

public:
    const BoxStateDomainTemplate<T,P,D> *box_domain;

    BoxDisjunctsDomainTemplate(const BoxStateDomainTemplate<T,P,D> *box_domain) { this->box_domain = box_domain; }

    virtual std::vector<std::pair<T, P>> filter(const T &training_set_abstraction, const P &predicate_abstraction) const = 0;
    virtual std::vector<std::pair<T, P>> filterNegated(const T &training_set_abstraction, const P &predicate_abstraction) const = 0;

    // TODO it is easy to imagine cases where the meetImpurity(Not)EqualsZero computations
    // should be able to split into individual disjuncts as well
    BoxDisjunctsType meetImpurityEqualsZero(const BoxDisjunctsType &element) const { return transformEachDisjunct(element, &BoxStateDomainTemplate<T,P,D>::meetImpurityEqualsZero); }
    BoxDisjunctsType meetImpurityNotEqualsZero(const BoxDisjunctsType &element) const { return transformEachDisjunct(element, &BoxStateDomainTemplate<T,P,D>::meetImpurityNotEqualsZero); }
    BoxDisjunctsType meetPhiIsBottom(const BoxDisjunctsType &element) const { return transformEachDisjunct(element, &BoxStateDomainTemplate<T,P,D>::meetPhiIsBottom); }
    BoxDisjunctsType meetPhiIsNotBottom(const BoxDisjunctsType &element) const { return transformEachDisjunct(element, &BoxStateDomainTemplate<T,P,D>::meetPhiIsNotBottom); }
    BoxDisjunctsType meetXModelsPhi(const BoxDisjunctsType &element, const FeatureVector &x) const { return transformEachDisjunct(element, &BoxStateDomainTemplate<T,P,D>::meetXModelsPhi, x); }
    BoxDisjunctsType meetXNotModelsPhi(const BoxDisjunctsType &element, const FeatureVector &x) const { return transformEachDisjunct(element, &BoxStateDomainTemplate<T,P,D>::meetXNotModelsPhi, x); }
    BoxDisjunctsType applyBestSplit(const BoxDisjunctsType &element) const { return transformEachDisjunct(element, &BoxStateDomainTemplate<T,P,D>::applyBestSplit); }
    BoxDisjunctsType applySummary(const BoxDisjunctsType &element) const { return transformEachDisjunct(element, &BoxStateDomainTemplate<T,P,D>::applySummary); }
    BoxDisjunctsType applyFilter(const BoxDisjunctsType &element) const { return filterAndUnion(element, false); }
    BoxDisjunctsType applyFilterNegated(const BoxDisjunctsType &element) const { return filterAndUnion(element, true); }


    bool isBottomElement(const BoxDisjunctsType &element) const;
    BoxDisjunctsType binary_join(const BoxDisjunctsType &e1, const BoxDisjunctsType &e2) const;
    BoxDisjunctsType join(const std::vector<BoxDisjunctsType> &elements) const; // We can override the aggregate to be more efficient
};


/**
 * BoxDisjunctsDomain member functions.
 */

template <typename T, typename P, typename D>
inline typename BoxDisjunctsDomainTemplate<T,P,D>::BoxDisjunctsType BoxDisjunctsDomainTemplate<T,P,D>::transformEachDisjunct(const BoxDisjunctsType &element, SingleBoxType (BoxStateDomainTemplate<T,P,D>::*fptr)(const SingleBoxType&) const) const {
    std::vector<SingleBoxType> ret;
    for(auto i = element.cbegin(); i != element.cend(); i++) {
        SingleBoxType temp = (box_domain->*fptr)(*i);
        if(!box_domain->isBottomElement(temp)) {
            ret.push_back(temp);
        }
    }
    return ret;
}

template <typename T, typename P, typename D>
inline typename BoxDisjunctsDomainTemplate<T,P,D>::BoxDisjunctsType BoxDisjunctsDomainTemplate<T,P,D>::transformEachDisjunct(const BoxDisjunctsType &element, SingleBoxType (BoxStateDomainTemplate<T,P,D>::*fptr)(const SingleBoxType&, const FeatureVector&) const, const FeatureVector &x) const {
    std::vector<SingleBoxType> ret;
    for(auto i = element.cbegin(); i != element.cend(); i++) {
        SingleBoxType temp = (box_domain->*fptr)(*i, x);
        if(!box_domain->isBottomElement(temp)) {
            ret.push_back(temp);
        }
    }
    return ret;
}

template <typename T, typename P, typename D>
typename BoxDisjunctsDomainTemplate<T,P,D>::BoxDisjunctsType BoxDisjunctsDomainTemplate<T,P,D>::filterAndUnion(const BoxDisjunctsType &element, bool negated) const {
    std::vector<SingleBoxType> ret;
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
            SingleBoxType temp_box = {j->first, j->second, i->posterior_distribution_abstraction};
            if(!box_domain->isBottomElement(temp_box)) {
                ret.push_back(temp_box);
            }
        }
    }
    return ret;
}

template <typename T, typename P, typename D>
bool BoxDisjunctsDomainTemplate<T,P,D>::isBottomElement(const BoxDisjunctsType &element) const {
    return element.size() == 0; // XXX could/should check that each disjunct is non-bot
}

template <typename T, typename P, typename D>
typename BoxDisjunctsDomainTemplate<T,P,D>::BoxDisjunctsType BoxDisjunctsDomainTemplate<T,P,D>::binary_join(const BoxDisjunctsType &e1, const BoxDisjunctsType &e2) const {
    BoxDisjunctsType ret = e1;
    for(auto i = e2.cbegin(); i != e2.cend(); i++) {
        ret.push_back(*i);
    }
    return ret;
}

template <typename T, typename P, typename D>
typename BoxDisjunctsDomainTemplate<T,P,D>::BoxDisjunctsType BoxDisjunctsDomainTemplate<T,P,D>::join(const std::vector<BoxDisjunctsType> &elements) const {
    BoxDisjunctsType ret;
    for(auto i = elements.cbegin(); i != elements.cend(); i++) {
        for(auto j = i->cbegin(); j != i->cend(); j++) {
            ret.push_back(*j);
        }
    }
    return ret;
}

#endif
