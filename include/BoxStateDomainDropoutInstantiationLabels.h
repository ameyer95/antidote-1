#ifndef BOXSTATEDOMAINDROPOUTINSTANTIATIONLABELS_H
#define BOXSTATEDOMAINDROPOUTINSTANTIATIONLABELS_H

#include "BoxStateDomainTemplateLabels.hpp"
#include "CategoricalDistribution.hpp"
#include "DataReferences.h"
#include "Feature.hpp"
#include "Interval.h"
#include "SymbolicPredicate.hpp"
#include <list>
#include <optional>
#include <utility>
#include <vector>


/**
 * We have the individual types for each component of the state tuple:
 *   - Training set abstraction: we will consider a fixed training set
 *     with up to n-many elements missing
 *   - Predicate abstraction: we will consider a finite set of (symbolic) predicates,
 *     each individually similar to the Predicate.hpp Predicate class
 *   - Posterior distribution abstraction: we will have a categorical
 *     probability density function where each value is an interval
 */


class TrainingReferencesWithDropoutLabels {
public:
    struct DropoutCounts {
        std::vector<int> counts;
        int num_dropout;
    };

    DataReferences training_references;
    int num_dropout;

    TrainingReferencesWithDropoutLabels() {} // Constructed like this should be a bottom element
    TrainingReferencesWithDropoutLabels(DataReferences training_references, int num_dropout);

    std::vector<int> baseCounts() const;
    std::pair<DropoutCounts, DropoutCounts> splitCounts(const SymbolicPredicate &phi) const;
    TrainingReferencesWithDropoutLabels pureSetRestriction(std::list<int> pure_possible_classes) const;
    TrainingReferencesWithDropoutLabels filter(const SymbolicPredicate &phi, bool positive_flag) const; // Returns a new object
};


typedef std::vector<std::optional<SymbolicPredicate>> PredicateAbstractionLabels;

typedef CategoricalDistribution<Interval<double>> PosteriorDistributionAbstractionLabels;


/**
 * Next, we have the individual consituent domains
 */


class TrainingSetDropoutDomainLabels : public TrainingSetDomainTemplateLabels<TrainingReferencesWithDropoutLabels> {
public:
    TrainingReferencesWithDropoutLabels meetImpurityEqualsZero(const TrainingReferencesWithDropoutLabels &element) const;
    TrainingReferencesWithDropoutLabels meetImpurityNotEqualsZero(const TrainingReferencesWithDropoutLabels &element) const;

    bool isBottomElement(const TrainingReferencesWithDropoutLabels &element) const;
    TrainingReferencesWithDropoutLabels binary_join(const TrainingReferencesWithDropoutLabels &e1, const TrainingReferencesWithDropoutLabels &e2) const;
};


class PredicateSetDomainLabels : public PredicateDomainTemplateLabels<PredicateAbstractionLabels> {
public:
    PredicateAbstractionLabels meetPhiIsBottom(const PredicateAbstractionLabels &element) const;
    PredicateAbstractionLabels meetPhiIsNotBottom(const PredicateAbstractionLabels &element) const;
    PredicateAbstractionLabels meetXModelsPhi(const PredicateAbstractionLabels &element, const FeatureVector &x) const;
    PredicateAbstractionLabels meetXNotModelsPhi(const PredicateAbstractionLabels &element, const FeatureVector &x) const;

    bool isBottomElement(const PredicateAbstractionLabels &element) const;
    PredicateAbstractionLabels binary_join(const PredicateAbstractionLabels &e1, const PredicateAbstractionLabels &e2) const;
};


class PosteriorDistributionIntervalDomainLabels : public PosteriorDistributionDomainTemplateLabels<PosteriorDistributionAbstractionLabels> {
public:
    bool isBottomElement(const PosteriorDistributionAbstractionLabels &element) const;
    PosteriorDistributionAbstractionLabels binary_join(const PosteriorDistributionAbstractionLabels &e1, const PosteriorDistributionAbstractionLabels &e2) const;
};


/**
 * Finally, the remaining box domain subclass
 */


class BoxDropoutDomainLabels : public BoxStateDomainTemplateLabels<TrainingReferencesWithDropoutLabels, PredicateAbstractionLabels, PosteriorDistributionAbstractionLabels> {
private:
    typedef std::pair<SymbolicPredicate, Interval<double>> ScoreEntry;
    void computePredicatesAndScores(
            std::list<ScoreEntry> &exists_nontrivial,
            std::list<const ScoreEntry *> &forall_nontrivial,
            const TrainingReferencesWithDropoutLabels &training_set_abstraction,
            int feature_index ) const;
    void computeBooleanFeaturePredicateAndScore(
            std::list<ScoreEntry> &exists_nontrivial,
            std::list<const ScoreEntry *> &forall_nontrivial,
            const TrainingReferencesWithDropoutLabels &training_set_abstraction,
            int feature_index ) const;
    void computeNumericFeaturePredicatesAndScores(
            std::list<ScoreEntry> &exists_nontrivial,
            std::list<const ScoreEntry *> &forall_nontrivial,
            const TrainingReferencesWithDropoutLabels &training_set_abstraction,
            int feature_index ) const;

public:
    using BoxStateDomainTemplateLabels::BoxStateDomainTemplateLabels; // Inherit the constructor

    PredicateAbstractionLabels bestSplit(const TrainingReferencesWithDropoutLabels &training_set_abstraction) const;
    TrainingReferencesWithDropoutLabels filter(const TrainingReferencesWithDropoutLabels &training_set_abstraction, const PredicateAbstractionLabels &predicate_abstraction) const;
    TrainingReferencesWithDropoutLabels filterNegated(const TrainingReferencesWithDropoutLabels &training_set_abstraction, const PredicateAbstractionLabels &predicate_abstraction) const;
    PosteriorDistributionAbstractionLabels summary(const TrainingReferencesWithDropoutLabels &training_set_abstraction) const;
};


#endif
