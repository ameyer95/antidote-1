#ifndef INFORMATION_MATH_H
#define INFORMATION_MATH_H

#include "Interval.h"
#include <utility>

double estimateBernoulli(int num_zeros, int num_ones);
Interval<double> estimateBernoulli(int num_zeros, int num_ones, int num_dropout);

// Computes the Gini Impurity
double impurity(const std::pair<int, int> &counts);
Interval<double> impurity(const std::pair<int, int> &counts, int num_dropout);

// These are non-normalized
double jointImpurity(const std::pair<int, int> &counts1, const std::pair<int, int> &counts2);
Interval<double> jointImpurity(const std::pair<int, int> &counts1, int num_dropout1,
                               const std::pair<int, int> &counts2, int num_dropout2);

double informationGain(const std::pair<int, int> &counts1, const std::pair<int, int> &counts2);

#endif
