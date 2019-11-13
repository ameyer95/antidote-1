#ifndef CONCRETETRAININGREFERENCES_H
#define CONCRETETRAININGREFERENCES_H

#include "CategoricalDistribution.hpp"
#include "DataReferences.h"
#include "DataSet.hpp"
#include "Predicate.hpp"
#include <list>
#include <optional>
#include <utility>
#include <vector>


// A mutable wrapper for DataReferences
// that implements some primitives for the concrete semantics
class ConcreteTrainingReferences {
private:
    DataReferences training_references;

    std::vector<int> sampleCounts() const;
    std::pair<std::vector<int>, std::vector<int>> splitCounts(const Predicate &phi) const;

    std::list<Predicate> gatherPredicates() const;
    void computePredicatesAndScores(std::list<std::pair<Predicate, double>> &store, int feature_index) const;
    void computeBooleanFeaturePredicateAndScore(std::list<std::pair<Predicate, double>> &store, int feature_index) const;
    void computeNumericFeaturePredicatesAndScores(std::list<std::pair<Predicate, double>> &store, int feature_index) const;

public:
    ConcreteTrainingReferences() {}
    ConcreteTrainingReferences(const DataSet *training_set) { training_references = DataReferences(training_set); }

    bool isPure() const;
    void filter(const Predicate &phi, bool mode);
    CategoricalDistribution<double> summary() const;
    std::optional<Predicate> bestSplit() const;
};


#endif
