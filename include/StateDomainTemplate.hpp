#ifndef STATEDOMAINTEMPLATE_HPP
#define STATEDOMAINTEMPLATE_HPP

#include "AbstractDomainTemplate.hpp"
#include <vector>


/**
 * An abstract class that sets up the abstract transformers
 * the our abstract semantics require of the state abstraction.
 *
 * The AbstractDomainTemplate base class is normally instantiated in pairs
 * with the AbstractElementTemplate base class;
 * since this is effectively just an interface,
 * any actual instantations of the state domain should be accompanied
 * by the instantiated abstract element.
 * Accordingly, the template parameter A should be that element subclass.
 */
template <typename A>
class StateDomainTemplate : public AbstractDomainTemplate<A> {
public:
    virtual A meetImpurityEqualsZero(const A &element) const = 0;
    virtual A meetImpurityNotEqualsZero(const A &element) const = 0;
    virtual A meetPhiIsBottom(const A &element) const = 0;
    virtual A meetPhiIsNotBottom(const A &element) const = 0;
    virtual A meetXModelsPhi(const A &element) const = 0;
    virtual A meetXNotModelsPhi(const A &element) const = 0;
    virtual A applyBestSplit(const A &element) const = 0;
    virtual A applySummary(const A &element) const = 0;
    virtual A applyFilter(const A &element) const = 0;
    virtual A applyFilterNegated(const A &element) const = 0;
};


#endif
