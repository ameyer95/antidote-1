#ifndef INFORMATION_MATH_H
#define INFORMATION_MATH_H

#include "Interval.h"
#include "CategoricalDistribution.hpp"
#include <vector>


// Give them names instead of having to assume a std::pair convention
struct BinarySamples {
    int num_zeros = 0;
    int num_ones = 0;
};

double estimateBernoulli(const BinarySamples &counts);
Interval<double> estimateBernoulli(const BinarySamples &counts, int num_dropout);

// Computes the Gini Impurity
double impurity(const BinarySamples &counts);
Interval<double> impurity(const BinarySamples &counts, int num_dropout);

// These are non-normalized
double jointImpurity(const BinarySamples &counts1, const BinarySamples &counts2);
Interval<double> jointImpurity(const BinarySamples &counts1, int num_dropout1,
                               const BinarySamples &counts2, int num_dropout2);

CategoricalDistribution<double> estimateCategorical(const std::vector<int> &counts);
double impurity(const std::vector<int> &counts);
double jointImpurity(const std::vector<int> &counts1, const std::vector<int> &counts2);

#endif
