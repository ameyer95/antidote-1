#ifndef BOXSTATEDOMAINTEMPLATEADDITION_HPP
#define BOXSTATEDOMAINTEMPLATEADDITION_HPP

/**
 * This file defines the basic "box" abstract domain for the program state.
 * In other words, it tries to break down the abstract transformers
 * into operations over each individual component of the state tuple.
 * Accordingly, this file first sets up a framework for abstract domains over each of
 *   - The training set abstraction
 *   - The predicate abstraction
 *   - The posterior distribution abstraction
 * all of which are then used in the final box domain class.
 */

#include "AbstractDomainTemplate.hpp"
#include "Feature.hpp"
#include "StateDomainTemplate.hpp"


/**
 * First, the constituent domains
 */

template <typename T>
class TrainingSetDomainTemplateAddition : public AbstractDomainTemplate<T> {
public:
    virtual T meetImpurityEqualsZero(const T &element) const = 0;
    virtual T meetImpurityNotEqualsZero(const T &element) const = 0;
};


template <typename P>
class PredicateDomainTemplateAddition : public AbstractDomainTemplate<P> {
public:
    virtual P meetPhiIsBottom(const P &element) const = 0;
    virtual P meetPhiIsNotBottom(const P &element) const = 0;
    virtual P meetXModelsPhi(const P &element, const FeatureVector &x) const = 0;
    virtual P meetXNotModelsPhi(const P &element, const FeatureVector &x) const = 0;
};


template <typename D>
class PosteriorDistributionDomainTemplateAddition : public AbstractDomainTemplate<D> {
public:
    // BoxStateDomain doesn't actually require anything special from this,
    // so we just repeat an existing pure virtual function.
    virtual bool isBottomElement(const D &element) const = 0;
};


/**
 * The actual box domain
 */

// TODO could this be replaced with std::tuple?
template <typename T, typename P, typename D>
struct BoxStateAbstractionAddition {
    T training_set_abstraction;
    P predicate_abstraction;
    D posterior_distribution_abstraction;
};


template <typename T, typename P, typename D>
class BoxStateDomainTemplateAddition : public StateDomainTemplate<BoxStateAbstractionAddition<T,P,D>> {
public:
    const TrainingSetDomainTemplateAddition<T> *training_set_domain;
    const PredicateDomainTemplateAddition<P> *predicate_domain;
    const PosteriorDistributionDomainTemplateAddition<D> *posterior_distribution_domain;

    BoxStateDomainTemplateAddition(const TrainingSetDomainTemplateAddition<T> *training_set_domain,
                           const PredicateDomainTemplateAddition<P> *predicate_domain,
                           const PosteriorDistributionDomainTemplateAddition<D> *posterior_distribution_domain);

    virtual P bestSplit(const T &training_set_abstraction) const = 0;
    virtual T filter(const T &training_set_abstraction, const P &predicate_abstraction) const = 0;
    virtual T filterNegated(const T &training_set_abstraction, const P &predicate_abstraction) const = 0;
    virtual D summary(const T &training_set_abstraction) const = 0;

    BoxStateAbstractionAddition<T,P,D> meetImpurityEqualsZero(const BoxStateAbstractionAddition<T,P,D> &element) const;
    BoxStateAbstractionAddition<T,P,D> meetImpurityNotEqualsZero(const BoxStateAbstractionAddition<T,P,D> &element) const;
    BoxStateAbstractionAddition<T,P,D> meetPhiIsBottom(const BoxStateAbstractionAddition<T,P,D> &element) const;
    BoxStateAbstractionAddition<T,P,D> meetPhiIsNotBottom(const BoxStateAbstractionAddition<T,P,D> &element) const;
    BoxStateAbstractionAddition<T,P,D> meetXModelsPhi(const BoxStateAbstractionAddition<T,P,D> &element, const FeatureVector &x) const;
    BoxStateAbstractionAddition<T,P,D> meetXNotModelsPhi(const BoxStateAbstractionAddition<T,P,D> &element, const FeatureVector &x) const;
    BoxStateAbstractionAddition<T,P,D> applyBestSplit(const BoxStateAbstractionAddition<T,P,D> &element) const;
    BoxStateAbstractionAddition<T,P,D> applySummary(const BoxStateAbstractionAddition<T,P,D> &element) const;
    BoxStateAbstractionAddition<T,P,D> applyFilter(const BoxStateAbstractionAddition<T,P,D> &element) const;
    BoxStateAbstractionAddition<T,P,D> applyFilterNegated(const BoxStateAbstractionAddition<T,P,D> &element) const;

    bool isBottomElement(const BoxStateAbstractionAddition<T,P,D> &element) const;
    BoxStateAbstractionAddition<T,P,D> binary_join(const BoxStateAbstractionAddition<T,P,D> &e1, const BoxStateAbstractionAddition<T,P,D> &e2) const;
};


/**
 * BoxStateDomainTemplate member function templates
 * (note that no code generation happens for this file)
 */

template <typename T, typename P, typename D>
inline BoxStateDomainTemplateAddition<T,P,D>::BoxStateDomainTemplateAddition(const TrainingSetDomainTemplateAddition<T> *training_set_domain, const PredicateDomainTemplateAddition<P> *predicate_domain, const PosteriorDistributionDomainTemplateAddition<D> *posterior_distribution_domain) {
    this->training_set_domain = training_set_domain;
    this->predicate_domain = predicate_domain;
    this->posterior_distribution_domain = posterior_distribution_domain;
}

template <typename T, typename P, typename D>
inline bool BoxStateDomainTemplateAddition<T,P,D>::isBottomElement(const BoxStateAbstractionAddition<T,P,D> &element) const {
    return (training_set_domain->isBottomElement(element.training_set_abstraction) ||
            predicate_domain->isBottomElement(element.predicate_abstraction) ||
            posterior_distribution_domain->isBottomElement(element.posterior_distribution_abstraction));
}

template <typename T, typename P, typename D>
BoxStateAbstractionAddition<T,P,D> BoxStateDomainTemplateAddition<T,P,D>::meetImpurityEqualsZero(const BoxStateAbstractionAddition<T,P,D> &element) const {
    return BoxStateAbstractionAddition<T,P,D> {
        training_set_domain->meetImpurityEqualsZero(element.training_set_abstraction),
        element.predicate_abstraction,
        element.posterior_distribution_abstraction
    };
}

template <typename T, typename P, typename D>
BoxStateAbstractionAddition<T,P,D> BoxStateDomainTemplateAddition<T,P,D>::meetImpurityNotEqualsZero(const BoxStateAbstractionAddition<T,P,D> &element) const {
    return BoxStateAbstractionAddition<T,P,D> {
        training_set_domain->meetImpurityNotEqualsZero(element.training_set_abstraction),
        element.predicate_abstraction,
        element.posterior_distribution_abstraction
    };
}

template <typename T, typename P, typename D>
BoxStateAbstractionAddition<T,P,D> BoxStateDomainTemplateAddition<T,P,D>::meetPhiIsBottom(const BoxStateAbstractionAddition<T,P,D> &element) const {
    return BoxStateAbstractionAddition<T,P,D> {
        element.training_set_abstraction,
        predicate_domain->meetPhiIsBottom(element.predicate_abstraction),
        element.posterior_distribution_abstraction
    };
}

template <typename T, typename P, typename D>
BoxStateAbstractionAddition<T,P,D> BoxStateDomainTemplateAddition<T,P,D>::meetPhiIsNotBottom(const BoxStateAbstractionAddition<T,P,D> &element) const {
    return BoxStateAbstractionAddition<T,P,D> {
        element.training_set_abstraction,
        predicate_domain->meetPhiIsNotBottom(element.predicate_abstraction),
        element.posterior_distribution_abstraction
    };
}

template <typename T, typename P, typename D>
BoxStateAbstractionAddition<T,P,D> BoxStateDomainTemplateAddition<T,P,D>::meetXModelsPhi(const BoxStateAbstractionAddition<T,P,D> &element, const FeatureVector &x) const {
    return BoxStateAbstractionAddition<T,P,D> {
        element.training_set_abstraction,
        predicate_domain->meetXModelsPhi(element.predicate_abstraction, x),
        element.posterior_distribution_abstraction
    };
}

template <typename T, typename P, typename D>
BoxStateAbstractionAddition<T,P,D> BoxStateDomainTemplateAddition<T,P,D>::meetXNotModelsPhi(const BoxStateAbstractionAddition<T,P,D> &element, const FeatureVector &x) const {
    return BoxStateAbstractionAddition<T,P,D> {
        element.training_set_abstraction,
        predicate_domain->meetXNotModelsPhi(element.predicate_abstraction, x),
        element.posterior_distribution_abstraction
    };
}

template <typename T, typename P, typename D>
BoxStateAbstractionAddition<T,P,D> BoxStateDomainTemplateAddition<T,P,D>::applyBestSplit(const BoxStateAbstractionAddition<T,P,D> &element) const {
    return BoxStateAbstractionAddition<T,P,D> {
        element.training_set_abstraction,
        bestSplit(element.training_set_abstraction),
        element.posterior_distribution_abstraction
    };
}

template <typename T, typename P, typename D>
BoxStateAbstractionAddition<T,P,D> BoxStateDomainTemplateAddition<T,P,D>::applySummary(const BoxStateAbstractionAddition<T,P,D> &element) const {
    return BoxStateAbstractionAddition<T,P,D> {
        element.training_set_abstraction,
        element.predicate_abstraction,
        summary(element.training_set_abstraction)
    };
}

template <typename T, typename P, typename D>
BoxStateAbstractionAddition<T,P,D> BoxStateDomainTemplateAddition<T,P,D>::applyFilter(const BoxStateAbstractionAddition<T,P,D> &element) const {
    return BoxStateAbstractionAddition<T,P,D> {
        filter(element.training_set_abstraction, element.predicate_abstraction),
        element.predicate_abstraction,
        element.posterior_distribution_abstraction
    };
}

template <typename T, typename P, typename D>
BoxStateAbstractionAddition<T,P,D> BoxStateDomainTemplateAddition<T,P,D>::applyFilterNegated(const BoxStateAbstractionAddition<T,P,D> &element) const {
    return BoxStateAbstractionAddition<T,P,D> {
        filterNegated(element.training_set_abstraction, element.predicate_abstraction),
        element.predicate_abstraction,
        element.posterior_distribution_abstraction
    };
}

template <typename T, typename P, typename D>
BoxStateAbstractionAddition<T,P,D> BoxStateDomainTemplateAddition<T,P,D>::binary_join(const BoxStateAbstractionAddition<T,P,D> &e1, const BoxStateAbstractionAddition<T,P,D> &e2) const {
    if(isBottomElement(e1)) {
        return e2;
    } else if(isBottomElement(e2)) {
        return e1;
    } else {
        return BoxStateAbstractionAddition<T,P,D> {
            training_set_domain->binary_join(e1.training_set_abstraction, e2.training_set_abstraction),
            predicate_domain->binary_join(e1.predicate_abstraction, e2.predicate_abstraction),
            posterior_distribution_domain->binary_join(e1.posterior_distribution_abstraction, e2.posterior_distribution_abstraction)
        };
    }
}

#endif
