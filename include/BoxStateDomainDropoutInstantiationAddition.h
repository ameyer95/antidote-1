#ifndef BOXSTATEDOMAINDROPOUTINSTANTIATIONADDITION_H
#define BOXSTATEDOMAINDROPOUTINSTANTIATIONADDITION_H

#include "BoxStateDomainTemplateAddition.hpp"
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


class TrainingReferencesWithDropoutAddition {
public:
    struct DropoutCounts {
        std::vector<int> counts;
        int num_dropout;
        int sensitive_feature;
        float protected_value;
    };

    struct DropoutCountsWithProtected {
        DropoutCounts dropout;
        std::vector<int> protected_counts;
    };

    DataReferences training_references;
    int num_dropout;
    int sensitive_feature;
    float protected_value;

    TrainingReferencesWithDropoutAddition() {} // Constructed like this should be a bottom element
    TrainingReferencesWithDropoutAddition(DataReferences training_references, int num_dropout, int sensitive_feature, float protected_value);

    std::vector<int> baseCounts() const;
    std::pair<DropoutCountsWithProtected, DropoutCountsWithProtected> splitCounts(const SymbolicPredicate &phi) const;
    TrainingReferencesWithDropoutAddition pureSetRestriction(std::list<int> pure_possible_classes) const;
    TrainingReferencesWithDropoutAddition filter(const SymbolicPredicate &phi, bool positive_flag) const; // Returns a new object
};


typedef std::vector<std::optional<SymbolicPredicate>> PredicateAbstractionAddition;

typedef CategoricalDistribution<Interval<double>> PosteriorDistributionAbstractionAddition;


/**
 * Next, we have the individual consituent domains
 */


class TrainingSetDropoutDomainAddition : public TrainingSetDomainTemplateAddition<TrainingReferencesWithDropoutAddition> {
public:
    TrainingReferencesWithDropoutAddition meetImpurityEqualsZero(const TrainingReferencesWithDropoutAddition &element) const;
    TrainingReferencesWithDropoutAddition meetImpurityNotEqualsZero(const TrainingReferencesWithDropoutAddition&element) const;

    bool isBottomElement(const TrainingReferencesWithDropoutAddition &element) const;
    TrainingReferencesWithDropoutAddition binary_join(const TrainingReferencesWithDropoutAddition &e1, const TrainingReferencesWithDropoutAddition &e2) const;
};


class PredicateSetDomainAddition : public PredicateDomainTemplateAddition<PredicateAbstractionAddition> {
public:
    PredicateAbstractionAddition meetPhiIsBottom(const PredicateAbstractionAddition &element) const;
    PredicateAbstractionAddition meetPhiIsNotBottom(const PredicateAbstractionAddition &element) const;
    PredicateAbstractionAddition meetXModelsPhi(const PredicateAbstractionAddition &element, const FeatureVector &x) const;
    PredicateAbstractionAddition meetXNotModelsPhi(const PredicateAbstractionAddition &element, const FeatureVector &x) const;

    bool isBottomElement(const PredicateAbstractionAddition &element) const;
    PredicateAbstractionAddition binary_join(const PredicateAbstractionAddition &e1, const PredicateAbstractionAddition &e2) const;
};


class PosteriorDistributionIntervalDomainAddition : public PosteriorDistributionDomainTemplateAddition<PosteriorDistributionAbstractionAddition> {
public:
    bool isBottomElement(const PosteriorDistributionAbstractionAddition &element) const;
    PosteriorDistributionAbstractionAddition binary_join(const PosteriorDistributionAbstractionAddition &e1, const PosteriorDistributionAbstractionAddition &e2) const;
};


/**
 * Finally, the remaining box domain subclass
 */


class BoxDropoutDomainAddition : public BoxStateDomainTemplateAddition<TrainingReferencesWithDropoutAddition, PredicateAbstractionAddition, PosteriorDistributionAbstractionAddition> {
private:
    typedef std::pair<SymbolicPredicate, Interval<double>> ScoreEntry;
    void computePredicatesAndScores(
            std::list<ScoreEntry> &exists_nontrivial,
            std::list<const ScoreEntry *> &forall_nontrivial,
            const TrainingReferencesWithDropoutAddition &training_set_abstraction,
            int feature_index ) const;
    void computeBooleanFeaturePredicateAndScore(
            std::list<ScoreEntry> &exists_nontrivial,
            std::list<const ScoreEntry *> &forall_nontrivial,
            const TrainingReferencesWithDropoutAddition &training_set_abstraction,
            int feature_index ) const;
    void computeNumericFeaturePredicatesAndScores(
            std::list<ScoreEntry> &exists_nontrivial,
            std::list<const ScoreEntry *> &forall_nontrivial,
            const TrainingReferencesWithDropoutAddition &training_set_abstraction,
            int feature_index ) const;

public:
    using BoxStateDomainTemplateAddition::BoxStateDomainTemplateAddition; // Inherit the constructor

    PredicateAbstractionAddition bestSplit(const TrainingReferencesWithDropoutAddition &training_set_abstraction) const;
    TrainingReferencesWithDropoutAddition filter(const TrainingReferencesWithDropoutAddition &training_set_abstraction, const PredicateAbstractionAddition &predicate_abstraction) const;
    TrainingReferencesWithDropoutAddition filterNegated(const TrainingReferencesWithDropoutAddition &training_set_abstraction, const PredicateAbstractionAddition &predicate_abstraction) const;
    PosteriorDistributionAbstractionAddition summary(const TrainingReferencesWithDropoutAddition &training_set_abstraction) const;
};


#endif
