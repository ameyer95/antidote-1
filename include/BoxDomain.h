#ifndef BOXDOMAIN_H
#define BOXDOMAIN_H

#include "AbstractDomain.h"
#include "StateDomain.h"


template <typename T, typename P, typename D>
class BoxStateAbstraction : public AbstractElement {
private:
    bool is_bottom_element_flag; // XXX handle this correctly

public:
    BoxStateAbstraction() { is_bottom_element_flag = false; }
    BoxStateAbstraction(const T &training_set_abstraction, const P &predicate_abstraction, const D &posterior_distribution_abstraction);

    T training_set_abstraction;
    P predicate_abstraction;
    D posterior_distribution_abstraction;

    bool isBottomElement() const { return is_bottom_element_flag; }
};


/**
 * The BoxStateDomain class (later in the file) relies on having
 * abstract domains over each of the state tuple's constituents.
 * (Additionally, it has some requirements that cannot be so cleanly isolated,
 * but it still helps break down the implementation tasks
 * by having some additional classes.)
 */


template <typename T>
class TrainingSetDomain : public AbstractDomain<T> {
public:
    virtual T meetImpurityEqualsZero(const T &element) const = 0;
    virtual T meetImpurityNotEqualsZero(const T &element) const = 0;
};


template <typename P>
class PredicateDomain : public AbstractDomain<P> {
public:
    virtual P abstractBottomPhi() const = 0;
    virtual P abstractNotBottomPhi() const = 0;
    virtual P meetXModelsPhi(const P &element) const = 0;
    virtual P meetXNotModelsPhi(const P &element) const = 0;
};


template <typename D>
class PosteriorDistributionDomain : public AbstractDomain<D> {
public:
    // BoxStateDomain doesn't actually require anything from this,
    // so we just repeat the pure virtual join from AbstractDomain
    virtual D join(const D &e1, const D &e2) const = 0;
};


/**
 * Template parameters:
 * A should be some BoxStateAbstraction<T, P, D>
 * LT should be some TrainingSetDomain<T>
 * LP should be some PredicateDomain<P>
 * LD should be some PosteriorDistributionDomain<D>
 */
template <typename A, typename LT, typename T, typename LP, typename P, typename LD, typename D>
class BoxStateDomain : public StateDomain<A> {
private:
    LT training_set_domain;
    LP predicate_domain;
    LD posterior_distribution_domain;

public:
    virtual P bestSplit(const T &training_set_abstraction) const = 0;
    virtual T filter(const T &training_set_abstraction, const P &predicate_abstraction) const = 0;
    virtual T filterNegated(const T &training_set_abstraction, const P &predicate_abstraction) const = 0;
    virtual D summary(const T &training_set_abstraction) const = 0;

    A meetImpurityEqualsZero(const A &element) const;
    A meetImpurityNotEqualsZero(const A &element) const;
    A meetPhiIsBottom(const A &element) const;
    A meetPhiIsNotBottom(const A &element) const;
    A meetXModelsPhi(const A &element) const;
    A meetXNotModelsPhi(const A &element) const;
    A applyBestSplit(const A &element) const;
    A applySummary(const A &element) const;
    A applyFilter(const A &element) const;
    A applyFilterNegated(const A &element) const;

    A join(const A &e1, const A &e2) const;
};


/**
 * BoxStateAbstraction definitions
 */

template <typename T, typename P, typename D>
BoxStateAbstraction<T,P,D>::BoxStateAbstraction(const T &training_set_abstraction, const P &predicate_abstraction, const D &posterior_distribution_abstraction) {
    this->training_set_abstraction = training_set_abstraction;
    this->predicate_abstraction = predicate_abstraction;
    this->posterior_distribution_abstraction = posterior_distribution_abstraction;

    // XXX this logic is incorrect:
    // 1) should check if the consituents are bottom elements
    // 2) becomes potentially incorrect if the constituents are mutated
    is_bottom_element_flag = false;
}


/**
 * BoxStateDomain definitions
 */

template <typename A, typename LT, typename T, typename LP, typename P, typename LD, typename D>
A BoxStateDomain<A,LT,T,LP,P,LD,D>::meetImpurityEqualsZero(const A &element) const {
    return A(training_set_domain.meetImpurityEqualsZero(element.training_set_abstraction),
             element.predicate_abstraction,
             element.posterior_distribution_abstraction);
}

template <typename A, typename LT, typename T, typename LP, typename P, typename LD, typename D>
A BoxStateDomain<A,LT,T,LP,P,LD,D>::meetImpurityNotEqualsZero(const A &element) const {
    return A(training_set_domain.meetImpurityNotEqualsZero(element.training_set_abstraction),
             element.predicate_abstraction,
             element.posterior_distribution_abstraction);
}

template <typename A, typename LT, typename T, typename LP, typename P, typename LD, typename D>
A BoxStateDomain<A,LT,T,LP,P,LD,D>::meetPhiIsBottom(const A &element) const {
    return A(element.training_set_abstraction,
             predicate_domain.abstractBottomPhi(),
             element.posterior_distribution_abstraction);
}

template <typename A, typename LT, typename T, typename LP, typename P, typename LD, typename D>
A BoxStateDomain<A,LT,T,LP,P,LD,D>::meetPhiIsNotBottom(const A &element) const {
    return A(element.training_set_abstraction,
             predicate_domain.abstractNotBottomPhi(),
             element.posterior_distribution_abstraction);
}

template <typename A, typename LT, typename T, typename LP, typename P, typename LD, typename D>
A BoxStateDomain<A,LT,T,LP,P,LD,D>::meetXModelsPhi(const A &element) const {
    return A(element.training_set_abstraction,
             predicate_domain.meetXModelsPhi(element.predicate_abstraction),
             element.posterior_distribution_abstraction);
}

template <typename A, typename LT, typename T, typename LP, typename P, typename LD, typename D>
A BoxStateDomain<A,LT,T,LP,P,LD,D>::meetXNotModelsPhi(const A &element) const {
    return A(element.training_set_abstraction,
             predicate_domain.meetXNotModelsPhi(element.predicate_abstraction),
             element.posterior_distribution_abstraction);
}

template <typename A, typename LT, typename T, typename LP, typename P, typename LD, typename D>
A BoxStateDomain<A,LT,T,LP,P,LD,D>::applyBestSplit(const A &element) const {
    return A(element.training_set_abstraction,
             bestSplit(element.training_set_abstraction),
             element.posterior_distribution_abstraction);
}

template <typename A, typename LT, typename T, typename LP, typename P, typename LD, typename D>
A BoxStateDomain<A,LT,T,LP,P,LD,D>::applySummary(const A &element) const {
    return A(element.training_set_abstraction,
             element.predicate_abstraction,
             summary(element.training_set_abstraction));
}

template <typename A, typename LT, typename T, typename LP, typename P, typename LD, typename D>
A BoxStateDomain<A,LT,T,LP,P,LD,D>::applyFilter(const A &element) const {
    return A(filter(element.training_set_abstraction, element.predicate_abstraction),
             element.predicate_abstraction,
             element.posterior_distribution_abstraction);
}

template <typename A, typename LT, typename T, typename LP, typename P, typename LD, typename D>
A BoxStateDomain<A,LT,T,LP,P,LD,D>::applyFilterNegated(const A &element) const {
    return A(filterNegated(element.training_set_abstraction, element.predicate_abstraction),
             element.predicate_abstraction,
             element.posterior_distribution_abstraction);
}

template <typename A, typename LT, typename T, typename LP, typename P, typename LD, typename D>
A BoxStateDomain<A,LT,T,LP,P,LD,D>::join(const A &e1, const A &e2) const {
    if(e1.isBottomElement()) {
        return e2;
    } else if(e2.isBottomElement()) {
        return e1;
    } else {
        return A(training_set_domain.join(e1.training_set_abstraction, e2.training_set_abstraction),
                 predicate_domain.join(e1.predicate_abstraction, e2.predicate_abstraction),
                 posterior_distribution_domain.join(e1.posterior_distribution_abstraction, e2.posterior_distribution_abstraction));
    }
}

#endif
