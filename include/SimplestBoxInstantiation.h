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


class BooleanDropoutSet : public AbstractElement { // TODO properly handle isBottomElement etc
private:
    bool bottom_element_flag;

public:
    DataReferences<BooleanXYPair> training_set;
    int num_dropout;

    BooleanDropoutSet(); // Initializer as a bottom element
    BooleanDropoutSet(DataReferences<BooleanXYPair> training_set, int num_dropout);

    std::pair<int, int> baseCounts() const; // Returns a pair of (0 counts, 1 counts) not accounting for num_dropout
    BooleanDropoutSet pureSet(bool classification) const;

    bool isBottomElement() const { return bottom_element_flag; }
};


class BitvectorPredicateAbstraction : public AbstractElement { // TODO properly handle isBottomElement
private:
    bool bottom_element_flag;

public:
    std::vector<std::optional<int>> predicates;

    BitvectorPredicateAbstraction(); // Initializer as a bottom element
    BitvectorPredicateAbstraction(const std::vector<std::optional<int>> &predicates);

    bool isBottomElement() const { return bottom_element_flag; }
};
// Training set abstraction will be a DataReferences<BooleanXYPair> coupled with an integer
// (denoting how many elements could be missing from the set);
// Predicate abstraction will be a finite set of possibilities, so BitvectorPredicateAbstraction;
// Posterior abstraction will be a single interval of the possible Bernoulli values, so Interval<double>.
// Note that for everything except Interval<double>, we ended up making a subclass of AbstractElement.
// Interval<>::isEmpty takes the place of isBottomElement,
// and Interval<>::join is already defined in Interval.h, but at some point we may refactor this.

typedef BoxStateAbstraction<BooleanDropoutSet, BitvectorPredicateAbstraction, Interval<double>> SimplestBoxAbstraction;


/**
 * We need to implement each of the constituent domains
 */


class BooleanDropoutDomain : public TrainingSetDomain<BooleanDropoutSet> {
public:
    BooleanDropoutSet meetImpurityEqualsZero(const BooleanDropoutSet &element) const;
    BooleanDropoutSet meetImpurityNotEqualsZero(const BooleanDropoutSet &element) const;

    BooleanDropoutSet binary_join(const BooleanDropoutSet &e1, const BooleanDropoutSet &e2) const;
};


class BitvectorPredicateDomain : public PredicateDomain<BitvectorPredicateAbstraction> {
private:
    int num_X_indices;
    std::vector<bool> x; // The fixed input to the program

public:
    BitvectorPredicateDomain(int num_X_indices, const std::vector<bool> &x);

    BitvectorPredicateAbstraction abstractBottomPhi() const;
    BitvectorPredicateAbstraction abstractNotBottomPhi() const;
    BitvectorPredicateAbstraction meetXModelsPhi(const BitvectorPredicateAbstraction &element) const;
    BitvectorPredicateAbstraction meetXNotModelsPhi(const BitvectorPredicateAbstraction &element) const;

    BitvectorPredicateAbstraction binary_join(const BitvectorPredicateAbstraction &e1, const BitvectorPredicateAbstraction &e2) const;
};


class SingleIntervalDomain : public PosteriorDistributionDomain<Interval<double>> {
public:
    Interval<double> binary_join(const Interval<double> &e1, const Interval<double> &e2) const;
};


/**
 * Finally, the actual box domain
 */


class SimplestBoxDomain : public BoxStateDomain<SimplestBoxAbstraction, BooleanDropoutDomain, BooleanDropoutSet, BitvectorPredicateDomain, BitvectorPredicateAbstraction, SingleIntervalDomain, Interval<double>> {
public:
    BitvectorPredicateAbstraction bestSplit(const BooleanDropoutSet &training_set_abstraction) const;
    BooleanDropoutSet filter(const BooleanDropoutSet &training_set_abstraction, const BitvectorPredicateAbstraction &predicate_abstraction) const;
    BooleanDropoutSet filterNegated(const BooleanDropoutSet &training_set_abstraction, const BitvectorPredicateAbstraction &predicate_abstraction) const;
    Interval<double> summary(const BooleanDropoutSet &training_set_abstraction) const;
};


#endif
