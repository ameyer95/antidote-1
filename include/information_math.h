#ifndef INFORMATION_MATH_H
#define INFORMATION_MATH_H

#include "Interval.h"
#include <utility>
using namespace std;

// Computes the Gini Impurity
double impurity(const pair<int, int> &counts);
Interval<double> impurity(const pair<int, int> &counts, int num_dropout);

// These are non-normalized
double jointImpurity(const pair<int, int> &counts1, const pair<int, int> &counts2);
Interval<double> jointImpurity(const pair<int, int> &counts1, int num_dropout1,
                               const pair<int, int> &counts2, int num_dropout2);

double informationGain(const pair<int, int> &counts1, const pair<int, int> &counts2);

#endif
