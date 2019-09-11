#ifndef STATEDOMAIN_H
#define STATEDOMAIN_H

#include <vector>


template <typename T, typename P, typename D>
class StateAbstraction {
public:
    bool isBottomElement() const;
};


// E should be some StateAbstraction<T, P, D>
template <typename E, typename T, typename P, typename D>
class StateDomain {
public:
    E meetImpurityEqualsZero(const E &element) const;
    E meetImpurityNotEqualsZero(const E &element) const;
    E meetPhiIsBottom(const E &element) const;
    E meetPhiIsNotBottom(const E &element) const;
    E meetXModelsPhi(const E &element) const;
    E meetXNotModelsPhi(const E &element) const;
    E applyBestSplit(const E &element) const;
    E applySummary(const E &element) const;
    E applyFilter(const E &element) const;
    E applyFilterNegated(const E &element) const;

    E join(const E &e1, const E &e2) const;
    E join(const std::vector<E> &elements) const; 
};


#endif
