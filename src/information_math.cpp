#include "information_math.h"
#include "CategoricalDistribution.hpp"
#include "Interval.h"
#include <algorithm> // for std::max/min
#include <numeric> // for std::accumulate (easy summations)
#include <utility>
#include <vector>
using namespace std;

double estimateBernoulli(const BinarySamples &counts) {
    return (double)counts.num_ones / (counts.num_zeros + counts.num_ones);
}

Interval<double> estimateBernoulli(const BinarySamples &counts, int num_dropout) {
    // When num_dropout >= num_zeros + num_ones, anything is possible.
    // In the == case, this is because we assume estimating from an empty set is undefined behavior.
    if(counts.num_zeros + counts.num_ones <= num_dropout) {
        return Interval<double>(0, 1);
    }

    // Since this is effectively computing an average of a collection of 0s and 1s,
    // extremal behavior occurs either when maximally many 1s are removed
    // or when maximally many 0s are removed.
    // (This is more precise than the obvious count-interval division)
    BinarySamples minimizer, maximizer;
    minimizer.num_zeros = counts.num_zeros;
    minimizer.num_ones = max(0, counts.num_ones - num_dropout);
    maximizer.num_zeros = max(0, counts.num_zeros - num_dropout);
    maximizer.num_ones = counts.num_ones;

    return Interval<double>(estimateBernoulli(minimizer), estimateBernoulli(maximizer));
}

double impurity(const BinarySamples &counts) {
    double p = estimateBernoulli(counts);
    return p * (1-p) / 2; // Gini impurity
}

Interval<double> impurity(const BinarySamples &counts, int num_dropout) {
    Interval<double> p = estimateBernoulli(counts, num_dropout);
    return p * (Interval<double>(1) - p) * Interval<double>(2);
}

double jointImpurity(const BinarySamples &counts1, const BinarySamples &counts2) {
    return (counts1.num_zeros + counts1.num_ones) * impurity(counts1)
        + (counts2.num_zeros + counts2.num_ones) * impurity(counts2);
}

Interval<double> jointImpurity(const BinarySamples &counts1, int num_dropout1, const BinarySamples &counts2, int num_dropout2) {
    int total1 = counts1.num_zeros + counts1.num_ones;
    int total2 = counts2.num_zeros + counts2.num_ones;
    Interval<double> size1(total1 - num_dropout1, total1);
    Interval<double> size2(total2 - num_dropout2, total2);
    return size1 * impurity(counts1, num_dropout1) + size2 * impurity(counts2, num_dropout2);
}

CategoricalDistribution<double> estimateCategorical(const std::vector<int> &counts) {
    CategoricalDistribution<double> ret(counts.size());
    int total = accumulate(counts.begin(), counts.end(), 0);
    for(unsigned int i = 0; i < counts.size(); i++) {
        ret[i] = (double)counts[i] / total;
    }
    return ret;
}

double impurity(const vector<int> &counts) {
    CategoricalDistribution<double> p = estimateCategorical(counts);
    double total = 0;
    for(unsigned int i = 0; i < p.size(); i++) {
        total += p[i] * (1 - p[i]);
    }
    return total;
}

double jointImpurity(const vector<int> &counts1, const vector<int> &counts2) {
    int total1 = accumulate(counts1.begin(), counts1.end(), 0);
    int total2 = accumulate(counts2.begin(), counts2.end(), 0);
    return total1 * impurity(counts1) + total2 * impurity(counts2);
}

CategoricalDistribution<Interval<double>> estimateCategorical(const std::vector<int> &counts, int num_dropout) {
    // TODO
}

Interval<double> jointImpurity(const std::vector<int> &counts1, int num_dropout1, const std::vector<int> &counts2, int num_dropout2) {
    // TODO
}
