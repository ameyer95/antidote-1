#ifndef INFORMATION_MATH_H
#define INFORMATION_MATH_H

#include <utility>
using namespace std;

// Computes the Gini Impurity
double impurity(const pair<int, int> &counts);

double informationGain(const pair<int, int> &counts1, const pair<int, int> &counts2);

#endif
