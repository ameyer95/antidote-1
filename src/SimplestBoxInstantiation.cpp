#include "SimplestBoxInstantiation.h"
#include "Interval.h"
#include <optional>
#include <utility>
#include <vector>

/**
 * Some common primitives for the training-set-with-n-elements-missing
 * (more generally, its member functions...)
 */

BooleanDropoutSet::BooleanDropoutSet() {
    bottom_element_flag = true;
}

BooleanDropoutSet::BooleanDropoutSet(DataReferences<BooleanXYPair> training_set, int num_dropout) {
    this->training_set = training_set;
    this->num_dropout = num_dropout;
    bottom_element_flag = false; // XXX this is only correct when the passed parameters behave nicely
}

std::pair<int, int> BooleanDropoutSet::baseCounts() const {
    std::pair<int, int> counts;
    for(int i = 0; i < training_set.size(); i++) {
        if(training_set[i].second) {
            counts.second++;
        } else {
            counts.first++;
        }
    }
    return counts;
}

BooleanDropoutSet BooleanDropoutSet::pureSet(bool classification) const {
    DataReferences<BooleanXYPair> training_set_copy = training_set;
    int num_removed = 0;
    for(unsigned int i = 0; i < training_set_copy.size(); i++) {
        if(training_set_copy[i].second != classification) {
            training_set_copy.remove(i);
            num_removed++;
            i--;
        }
    }
    // We will only call this when it's guaranteed to be non-trivial,
    // so we need not check that num_removed <= num_dropout
    return BooleanDropoutSet(training_set_copy, num_dropout - num_removed);
}

/**
 * Constituent domain member functions
 */

BooleanDropoutSet BooleanDropoutDomain::meetImpurityEqualsZero(const BooleanDropoutSet &element) const {
    if(element.isBottomElement()) {
        return element;
    }
    std::pair<int, int> counts = element.baseCounts();
    bool pure_0_possible = false, pure_1_possible = false;
    if(counts.first <= element.num_dropout) {
        // It's possible that all of the 0-classification elements could be removed
        pure_1_possible = true;
    }
    if(counts.second <= element.num_dropout) {
        pure_0_possible = true;
    }

    // If neither are possible, we return a bottom element
    if(!pure_0_possible && !pure_1_possible) {
        return BooleanDropoutSet();
    }
    // If both are possible, our abstraction is too coarse to do anything precise,
    // so we will overapproximate the meet by returning the current value
    if(pure_0_possible && pure_1_possible) {
        return element;
    }
    // If only one is possible, we restrict the base training set accordingly (with an updated num_dropout)
    if(pure_0_possible) {
        return element.pureSet(false);
    } else { // when pure_1_possible
        return element.pureSet(true);
    }
}

BooleanDropoutSet BooleanDropoutDomain::meetImpurityNotEqualsZero(const BooleanDropoutSet &element) const {
    // Our abstraction is not capable of expressing this precisely
    return element;
}

BooleanDropoutSet BooleanDropoutDomain::binary_join(const BooleanDropoutSet &e1, const BooleanDropoutSet &e2) const {
    if(e1.isBottomElement()) {
        return e2;
    } else if(e2.isBottomElement()) {
        return e1;
    }
    DataReferences<BooleanXYPair> d = DataReferences<BooleanXYPair>::set_union(e1.training_set, e2.training_set);
    int n1 = d.size() - e2.training_set.size() + e2.num_dropout; // Note |(T1 U T2) \ T1| = |T2 \ T1|
    int n2 = d.size() - e1.training_set.size() + e1.num_dropout;
    return BooleanDropoutSet(d, std::max(n1, n2));
}

std::vector<std::optional<int>> BitvectorPredicateDomain::abstractBottomPhi() const {
    // TODO
}

std::vector<std::optional<int>> BitvectorPredicateDomain::abstractNotBottomPhi() const {
    // TODO
}

std::vector<std::optional<int>> BitvectorPredicateDomain::meetXModelsPhi(const std::vector<std::optional<int>> &element) const {
    // TODO
}

std::vector<std::optional<int>> BitvectorPredicateDomain::meetXNotModelsPhi(const std::vector<std::optional<int>> &element) const {
    // TODO
}

std::vector<std::optional<int>> BitvectorPredicateDomain::binary_join(const std::vector<std::optional<int>> &e1, const std::vector<std::optional<int>> &e2) const {
    // TODO
}

Interval<double> SingleIntervalDomain::binary_join(const Interval<double> &e1, const Interval<double> &e2) const {
    // TODO
}

/**
 * Actual box domain instantiation member functions
 */


std::vector<std::optional<int>> SimplestBoxDomain::bestSplit(const BooleanDropoutSet &training_set_abstraction) const {
    // TODO
}

BooleanDropoutSet SimplestBoxDomain::filter(const BooleanDropoutSet &training_set_abstraction, const std::vector<std::optional<int>> &predicate_abstraction) const {
    // TODO
}

BooleanDropoutSet SimplestBoxDomain::filterNegated(const BooleanDropoutSet &training_set_abstraction, const std::vector<std::optional<int>> &predicate_abstraction) const {
    // TODO
}

Interval<double> SimplestBoxDomain::summary(const BooleanDropoutSet &training_set_abstraction) const {
    // TODO
}
