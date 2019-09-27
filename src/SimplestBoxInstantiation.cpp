#include "SimplestBoxInstantiation.h"
#include "Interval.h"
#include <optional>
#include <vector>

/**
 * Consituent domain member functions
 */

BooleanDropoutSet BooleanDropoutDomain::meetImpurityEqualsZero(const BooleanDropoutSet &element) const {
    // TODO
}

BooleanDropoutSet BooleanDropoutDomain::meetImpurityNotEqualsZero(const BooleanDropoutSet &element) const {
    // TODO
}

BooleanDropoutSet BooleanDropoutDomain::binary_join(const BooleanDropoutSet &e1, const BooleanDropoutSet &e2) const {
    // TODO
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
