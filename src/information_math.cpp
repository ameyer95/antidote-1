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

Interval<double> estimateBernoulli(const BinarySamples &counts, int num_dropout, int num_add, int num_labels_flip, int num_features_flip) {
    // When num_dropout >= num_zeros + num_ones, anything is possible.
    // In the == case, this is because we assume estimating from an empty set is undefined behavior.
    if(counts.num_zeros + counts.num_ones <= num_dropout) {
        return Interval<double>(0, 1);
    }

    if (counts.num_zeros + counts.num_ones <= num_labels_flip) {
        return Interval<double>(0, 1);
    }

    // Since this is effectively computing an average of a collection of 0s and 1s,
    // extremal behavior occurs either when maximally many 1s are removed
    // or when maximally many 0s are removed.
    // (This is more precise than the obvious count-interval division)
    BinarySamples minimizer, maximizer;
    minimizer.num_zeros = min(counts.num_zeros + num_labels_flip + num_add, counts.num_zeros + counts.num_ones + num_add);
    minimizer.num_ones = max(0, counts.num_ones - num_dropout - num_labels_flip);
    maximizer.num_zeros = max(0, counts.num_zeros - num_dropout - num_labels_flip);
    maximizer.num_ones = min(counts.num_ones + num_labels_flip + num_add, counts.num_zeros + counts.num_ones + num_add);

    return Interval<double>(estimateBernoulli(minimizer), estimateBernoulli(maximizer));
}

double impurity(const BinarySamples &counts) {
    double p = estimateBernoulli(counts);
    return p * (1-p) / 2; // Gini impurity
}

Interval<double> impurity(const BinarySamples &counts, int num_dropout, int num_add, int num_labels_flip, int num_features_flip) {
    Interval<double> p = estimateBernoulli(counts, num_dropout, num_add, num_labels_flip, num_features_flip);
    return p * (Interval<double>(1) - p) * Interval<double>(2);
}

double jointImpurity(const BinarySamples &counts1, const BinarySamples &counts2) {
    return (counts1.num_zeros + counts1.num_ones) * impurity(counts1)
        + (counts2.num_zeros + counts2.num_ones) * impurity(counts2);
}

Interval<double> jointImpurity(const BinarySamples &counts1, int num_dropout1, int num_add1, int num_labels_flip1, int num_features_flip1,
                             const BinarySamples &counts2, int num_dropout2, int num_add2, int num_labels_flip2, int num_features_flip2) {
    int total1 = counts1.num_zeros + counts1.num_ones;
    int total2 = counts2.num_zeros + counts2.num_ones;
    Interval<double> size1(total1 - num_dropout1, total1 + num_add1);
    Interval<double> size2(total2 - num_dropout2, total2 + num_add2);
    return size1 * impurity(counts1, num_dropout1, num_add1, num_labels_flip1, num_features_flip2) + 
            size2 * impurity(counts2, num_dropout2, num_add2, num_labels_flip2, num_features_flip2);
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
    for(auto i = p.cbegin(); i != p.cend(); i++) {
        total += *i * (1 - *i);
    }
    return total;
}

double jointImpurity(const vector<int> &counts1, const vector<int> &counts2) {
    int total1 = accumulate(counts1.begin(), counts1.end(), 0);
    int total2 = accumulate(counts2.begin(), counts2.end(), 0);
    return total1 * impurity(counts1) + total2 * impurity(counts2);
}

CategoricalDistribution<Interval<double>> estimateCategorical(const std::vector<int> &counts, int num_dropout, int num_add, int num_labels_flip, int num_features_flip) {
    int count_total = accumulate(counts.cbegin(), counts.cend(), 0);
    // When num_dropout >= count_total, anything is possible.
    // In the == case, this is because we assume estimating from an empty set is undefined behavior.
    if(count_total <= num_dropout) {
        return CategoricalDistribution<Interval<double>>(counts.size(), Interval<double>(0,1));
    }

    if (count_total <= num_labels_flip) {
        return CategoricalDistribution<Interval<double>>(counts.size(), Interval<double>(0,1));
    }

    CategoricalDistribution<Interval<double>> ret(counts.size());
    // We estimate each component individually.
    // Since this is effectively computing an average of a collection of 0s and 1s,
    // extremal behavior occurs either when maximally many 1s are removed
    // or when maximally many 0s are removed.
    // (This is more precise than the obvious count-interval division)

    for(unsigned int i = 0; i < ret.size(); i++) {
        int count_not_y = count_total - counts[i];
        BinarySamples minimizer, maximizer;
        maximizer.num_ones = min(counts[i] + num_labels_flip + num_add, count_total + num_add);
        maximizer.num_zeros = max(0, count_not_y - num_dropout - num_labels_flip);
        minimizer.num_ones = max(0, counts[i] - num_dropout - num_labels_flip);
        minimizer.num_zeros = min(count_not_y + num_labels_flip + num_add, count_total + num_add);
        ret[i] = Interval<double>(estimateBernoulli(minimizer), estimateBernoulli(maximizer));
    }
    return ret; 
}

Interval<double> impurity(const std::vector<int> &counts, int num_dropout, int num_add, int num_labels_flip, int num_features_flip) {
    // TODO can be more precise
    CategoricalDistribution<Interval<double>> p = estimateCategorical(counts, num_dropout, num_add, num_labels_flip, num_features_flip);
    Interval<double> total(0);
    for(auto i = p.cbegin(); i != p.cend(); i++) {
        total = total + (*i * (Interval<double>(1) - *i));
    }
    return total;
}

Interval<double> jointImpurity(const std::vector<int> &counts1, int num_dropout1, int num_add1, int num_labels_flip1, int num_features_flip1,
                              const std::vector<int> &counts2, int num_dropout2, int num_add2, int num_labels_flip2, int num_features_flip2) {
    int total1 = accumulate(counts1.cbegin(), counts1.cend(), 0);
    int total2 = accumulate(counts2.cbegin(), counts2.cend(), 0);
    Interval<double> size1(total1 - num_dropout1, total1 + num_add1);
    Interval<double> size2(total2 - num_dropout2, total2 + num_add2);
    return size1 * impurity(counts1, num_dropout1, num_add1, num_labels_flip1, num_features_flip2) + 
            size2 * impurity(counts2, num_dropout2, num_add2, num_labels_flip2, num_features_flip2);
}