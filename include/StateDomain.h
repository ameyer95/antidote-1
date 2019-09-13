#ifndef STATEDOMAIN_H
#define STATEDOMAIN_H

#include "AbstractDomain.h"
#include <vector>


// A should be some appropriate AbstractElement class
template <typename A>
class StateDomain : public AbstractDomain<A> {
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
