#ifndef SIMPLESTBOXINSTANTIATION_H
#define SIMPLESTBOXINSTANTIATION_H

/**
 * This will be the simplest useful instantation of the abstraction framework
 * that I can thing of. Specifically, we will have:
 * - A training set whose elements are bitvectors (n bits of features, 1 bit of classification)
 * - Boolean predicates
 * - A Bernoulli posterior
 */

#include "BoxDomain.h"
#include "data_common.h"
#include "Interval.h"
#include <optional> // option types, requires c++17
#include <utility>
#include <vector>

typedef std::pair<std::vector<bool>, bool> BooleanXYPair;
// Thus our training set will be a DataReferences<BooleanXYPair>;
// Predicates are just integer indices or a NULL value, thus std::optional<int>
// Posterior is just a double.

typedef std::pair<DataReferences<BooleanXYPair>, int> BooleanDropoutSet;
// Training set abstraction will be a DataReferences<BooleanXYPair> coupled with an integer
// (denoting how many elements could be missing from the set);
// Predicate abstraction will be a finite set of possibilities, so std::vector<std::optional<int>>;
// Posterior abstraction will be a single interval of the possible Bernoulli values, so Interval<double>.

typedef BoxStateAbstraction<BooleanDropoutSet, std::vector<std::optional<int>>, Interval<double>> SimplestBoxAbstraction;


/**
 * We need to implement each of the constituent domains
 */


class BooleanDropoutDomain : public TrainingSetDomain<BooleanDropoutSet> {
public:
    BooleanDropoutSet meetImpurityEqualsZero(const BooleanDropoutSet &element) const;
    BooleanDropoutSet meetImpurityNotEqualsZero(const BooleanDropoutSet &element) const;

    BooleanDropoutSet binary_join(const BooleanDropoutSet &e1, const BooleanDropoutSet &e2) const;
};


class BitvectorPredicateDomain : public PredicateDomain<std::vector<std::optional<int>>> {
public:
    std::vector<std::optional<int>> abstractBottomPhi() const;
    std::vector<std::optional<int>> abstractNotBottomPhi() const;
    std::vector<std::optional<int>> meetXModelsPhi(const std::vector<std::optional<int>> &element) const;
    std::vector<std::optional<int>> meetXNotModelsPhi(const std::vector<std::optional<int>> &element) const;

    std::vector<std::optional<int>> binary_join(const std::vector<std::optional<int>> &e1, const std::vector<std::optional<int>> &e2) const;
};


class SingleIntervalDomain : public PosteriorDistributionDomain<Interval<double>> {
public:
    Interval<double> binary_join(const Interval<double> &e1, const Interval<double> &e2) const;
};


/**
 * Finally, the actual box domain
 */


class SimplestBoxDomain : public BoxStateDomain<SimplestBoxAbstraction, BooleanDropoutDomain, BooleanDropoutSet, BitvectorPredicateDomain, std::vector<std::optional<int>>, SingleIntervalDomain, Interval<double>> {
public:
    std::vector<std::optional<int>> bestSplit(const BooleanDropoutSet &training_set_abstraction) const;
    BooleanDropoutSet filter(const BooleanDropoutSet &training_set_abstraction, const std::vector<std::optional<int>> &predicate_abstraction) const;
    BooleanDropoutSet filterNegated(const BooleanDropoutSet &training_set_abstraction, const std::vector<std::optional<int>> &predicate_abstraction) const;
    Interval<double> summary(const BooleanDropoutSet &training_set_abstraction) const;
};


#endif
