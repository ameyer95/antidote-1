#include "information_math.h"
#include "CategoricalDistribution.hpp"
#include "Interval.h"
#include <algorithm> // for std::max/min
#include <numeric> // for std::accumulate (easy summations)
#include <utility>
#include <vector>
#include <iostream>
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
    for(auto i = p.cbegin(); i != p.cend(); i++) {
        total += *i * (1 - *i);
    }
    return total;
}

double impurityLabels(const vector<int> &counts) {
    CategoricalDistribution<double> p = estimateCategoricalLabels(counts);
    double total = 0;
    for (auto i = p.cbegin(); i!= p.cend(); i++) {
        total += *i * (1 - *i);
    }
    return total;
}

double jointImpurity(const vector<int> &counts1, const vector<int> &counts2) {
    int total1 = accumulate(counts1.begin(), counts1.end(), 0);
    int total2 = accumulate(counts2.begin(), counts2.end(), 0);
    return total1 * impurity(counts1) + total2 * impurity(counts2);
}

CategoricalDistribution<Interval<double>> estimateCategorical(const std::vector<int> &counts, int num_dropout) {
    int count_total = accumulate(counts.cbegin(), counts.cend(), 0);
    // When num_dropout >= count_total, anything is possible.
    // In the == case, this is because we assume estimating from an empty set is undefined behavior.
    if(count_total <= num_dropout) {
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
        maximizer.num_ones = counts[i];
        maximizer.num_zeros = max(0, count_not_y - num_dropout);
        minimizer.num_ones = max(0, counts[i] - num_dropout);
        minimizer.num_zeros = count_not_y;
        ret[i] = Interval<double>(estimateBernoulli(minimizer), estimateBernoulli(maximizer));
    }
    return ret;
}

Interval<double> impurity(const std::vector<int> &counts, int num_dropout) {
    // TODO can be more precise
    CategoricalDistribution<Interval<double>> p = estimateCategorical(counts, num_dropout);
    Interval<double> total(0);
    for(auto i = p.cbegin(); i != p.cend(); i++) {
        total = total + (*i * (Interval<double>(1) - *i));
    }
    return total;
}

Interval<double> jointImpurity(const std::vector<int> &counts1, int num_dropout1, const std::vector<int> &counts2, int num_dropout2) {
    int total1 = accumulate(counts1.cbegin(), counts1.cend(), 0);
    int total2 = accumulate(counts2.cbegin(), counts2.cend(), 0);
    Interval<double> size1(total1 - num_dropout1, total1);
    Interval<double> size2(total2 - num_dropout2, total2);
    return size1 * impurity(counts1, num_dropout1) + size2 * impurity(counts2, num_dropout2);
}

// ----------------------------------------------------------------------------------------
// LABEL FLIPPING
// ----------------------------------------------------------------------------------------

Interval<double> estimateBernoulliLabels(const BinarySamples &counts, int labels_to_flip) {
    // Only x entries left, and we can flip >x Obviously, return entire range (0,1)
    if (counts.num_zeros + counts.num_ones <= labels_to_flip) {
        return Interval<double>(0,1);
    }

    BinarySamples minimizer,maximizer;
    minimizer.num_zeros = min(counts.num_zeros + labels_to_flip, counts.num_zeros + counts.num_ones);
    minimizer.num_ones = max(0, counts.num_ones - labels_to_flip);
    maximizer.num_zeros = max(0, counts.num_zeros - labels_to_flip);
    maximizer.num_ones = min(counts.num_ones + labels_to_flip, counts.num_ones + counts.num_zeros);
    return Interval<double>(estimateBernoulli(minimizer), estimateBernoulli(maximizer));
}

CategoricalDistribution<Interval<double>> estimateCategoricalLabels(const std::vector<int> &counts, int labels_to_flip) {
    int count_total = accumulate(counts.cbegin(), counts.cend(), 0);
    if (count_total <= labels_to_flip) {
        return CategoricalDistribution<Interval<double>>(counts.size(),Interval<double>(0,1));
    }

    CategoricalDistribution<Interval<double>> ret(counts.size());
    for(unsigned int i = 0; i < ret.size(); i++) {
        int count_not_y = count_total - counts[i];
        BinarySamples minimizer, maximizer;
        maximizer.num_ones = min(count_total, counts[i] + labels_to_flip);
        maximizer.num_zeros = max(0, count_not_y - labels_to_flip);
        minimizer.num_ones = max(0, counts[i] - labels_to_flip);
        minimizer.num_zeros = min(count_not_y + labels_to_flip, count_total);
        ret[i] = Interval<double>(estimateBernoulli(minimizer), estimateBernoulli(maximizer));
    }

    return ret;
}

Interval<double> impurityLabels(const BinarySamples &counts, int labels_to_flip) {
    Interval<double> p = estimateBernoulliLabels(counts, labels_to_flip);
    return p * (Interval<double>(1) - p) * Interval<double>(2);
}

Interval<double> impurityLabels(const std::vector<int> &counts, int labels_to_flip) {
    CategoricalDistribution<Interval<double>> p = estimateCategoricalLabels(counts, labels_to_flip);
    Interval<double> total(0);
    for (auto i = p.cbegin(); i != p.cend(); i++) {
        total = total + (*i * (Interval<double>(1) - *i));
    }
    return total;
}

double jointImpurityLabels(const vector<int> &counts1, const vector<int> &counts2) {
    int total1 = accumulate(counts1.begin(), counts1.end(), 0);
    int total2 = accumulate(counts2.begin(), counts2.end(), 0);
    return total1 * impurityLabels(counts1) + total2 * impurityLabels(counts2);
}

Interval<double> jointImpurityLabels(const std::vector<int> &counts1, int labels_to_flip1, const std::vector<int> &counts2, int labels_to_flip2) {
    int total1 = accumulate(counts1.cbegin(), counts1.cend(), 0);
    int total2 = accumulate(counts2.cbegin(), counts2.cend(), 0);

    // Size 1 and Size 2 represent total size of the dataset - these won't change.
    Interval<double> size1(total1, total1);
    Interval<double> size2(total2, total2);
    return size1 * impurityLabels(counts1, labels_to_flip1) + size2 * impurityLabels(counts2, labels_to_flip2);
}

Interval<double> jointImpurityLabels(const BinarySamples &counts1, int labels_to_flip1, const BinarySamples &counts2, int labels_to_flip2) {
    int total1 = counts1.num_zeros + counts1.num_ones;
    int total2 = counts2.num_zeros + counts2.num_ones;
    Interval<double> size1(total1, total1);
    Interval<double> size2(total2, total2);
    return size1 * impurityLabels(counts1, labels_to_flip1) + size2 * impurityLabels(counts2, labels_to_flip2);
}

Interval<double> jointImpurityLabels(const BinarySamples &counts1, int labels_to_flip1, const BinarySamples &counts2, int labels_to_flip2, const BinarySamples &protected_counts1, const BinarySamples &protected_counts2) {
    int total1 = counts1.num_zeros + counts1.num_ones;
    int total2 = counts2.num_zeros + counts2.num_ones;
    int totalWeCanFlip1 = std::min(labels_to_flip1, protected_counts1.num_zeros);
    int totalWeCanFlip2 = std::min(labels_to_flip2, protected_counts2.num_zeros);
    
    Interval<double> size1(total1, total1);
    Interval<double> size2(total2, total2);
    return size1 * impurityLabels(counts1, totalWeCanFlip1) + size2 * impurityLabels(counts2, totalWeCanFlip2);
}

// ----------------------------------------------------------------------------------------
// LOPSIDED LABEL FLIPPING
// ----------------------------------------------------------------------------------------

Interval<double> estimateBernoulliLabelsLop(const BinarySamples &counts, int labels_to_flip) {
    // TO DO ANNA - edge cases where we can return (1,1) or (0,0)?
    if (counts.num_zeros + counts.num_ones <= labels_to_flip) {
        return Interval<double>(0,1);
    }

    BinarySamples minimizer,maximizer;
    // Under this model, we cannot create any new zero's
    minimizer.num_zeros = counts.num_zeros;
    // We can't decrease the # one's
    minimizer.num_ones = counts.num_ones;
    maximizer.num_zeros = max(0, counts.num_zeros - labels_to_flip);
    maximizer.num_ones = min(counts.num_ones + labels_to_flip, counts.num_ones + counts.num_zeros);
    return Interval<double>(estimateBernoulli(minimizer), estimateBernoulli(maximizer));
}

CategoricalDistribution<Interval<double>> estimateCategoricalLabelsLop(const std::vector<int> &counts, int labels_to_flip) {
    // lopsided label flipping
    int count_total = accumulate(counts.cbegin(), counts.cend(), 0);
    if (count_total <= labels_to_flip) {
        return CategoricalDistribution<Interval<double>>(counts.size(),Interval<double>(0,1));
    }

    CategoricalDistribution<Interval<double>> ret(counts.size());
    for (unsigned int i = 0; i < ret.size(); i++) {
        int count_not_y = count_total - counts[i];
        BinarySamples minimizer,maximizer;
        maximizer.num_ones = min(count_total, counts[i] + labels_to_flip);
        maximizer.num_zeros = max(0, count_not_y - labels_to_flip);
        minimizer.num_ones = counts[i];
        minimizer.num_zeros = count_not_y;
        ret[i] = Interval<double>(estimateBernoulli(minimizer), estimateBernoulli(maximizer));
    }

    return ret;
}

Interval<double> impurityLabelsLop(const BinarySamples &counts, int labels_to_flip) {
    // For one-sided label flipping
    Interval<double> p = estimateBernoulliLabelsLop(counts, labels_to_flip);
    return p * (Interval<double>(1) - p) * Interval<double>(2);
}

Interval<double> impurityLabelsLop(const std::vector<int> &counts, int labels_to_flip) {
   
    CategoricalDistribution<Interval<double>> p = estimateCategoricalLabelsLop(counts, labels_to_flip);
    Interval<double> total(0);
    for (auto i= p.cbegin(); i != p.cend(); i++) {
        total = total + (*i * (Interval<double>(1) - *i));
    }
    return total;
}

Interval<double> jointImpurityLabels(const std::vector<int> &counts1, int labels_to_flip1, const std::vector<int> &counts2, int labels_to_flip2, const std::vector<int> &protected_counts1, const std::vector<int> &protected_counts2) {
    int total1 = accumulate(counts1.cbegin(), counts1.cend(), 0);
    int total2 = accumulate(counts2.cbegin(), counts2.cend(), 0);
    int numWeCanFlip1 = std::min(labels_to_flip1, protected_counts1[0]);
    int numWeCanFlip2 = std::min(labels_to_flip2, protected_counts2[0]);
        
    Interval<double> size1(total1, total1);
    Interval<double> size2(total2, total2);

    return size1 * impurityLabelsLop(counts1, numWeCanFlip1) + size2 * impurityLabelsLop(counts2, numWeCanFlip2);
}

// ----------------------------------------------------------------------------------------
// DATA ADDITION
// ----------------------------------------------------------------------------------------

Interval<double> estimateBernoulliAddition(const BinarySamples &counts, int n) {
    // Only x entries left, and we can flip >x Obviously, return entire range (0,1)
    if (counts.num_zeros + counts.num_ones <= n) {
        return Interval<double>(0,1);
    }

    BinarySamples minimizer,maximizer;
    minimizer.num_zeros = counts.num_zeros + n;
    minimizer.num_ones = counts.num_ones;
    maximizer.num_zeros = counts.num_zeros;
    maximizer.num_ones = counts.num_ones + n;
    return Interval<double>(estimateBernoulli(minimizer), estimateBernoulli(maximizer));
}

CategoricalDistribution<Interval<double>> estimateCategoricalAddition(const std::vector<int> &counts, int n) {
    int count_total = accumulate(counts.cbegin(), counts.cend(), 0);
    if (count_total <= n) {
        return CategoricalDistribution<Interval<double>>(counts.size(),Interval<double>(0,1));
    }

    CategoricalDistribution<Interval<double>> ret(counts.size());
    for(unsigned int i = 0; i < ret.size(); i++) {
        int count_not_y = count_total - counts[i];
        BinarySamples minimizer, maximizer;
        maximizer.num_ones = counts[i] + n;
        maximizer.num_zeros = count_not_y;
        minimizer.num_ones = counts[i];
        minimizer.num_zeros = count_not_y + n;
        ret[i] = Interval<double>(estimateBernoulli(minimizer), estimateBernoulli(maximizer));
    }

    return ret;
}

Interval<double> impurityAddition(const std::vector<int> &counts, int n) {
    CategoricalDistribution<Interval<double>> p = estimateCategoricalAddition(counts, n);
    Interval<double> total(0);
    for (auto i = p.cbegin(); i != p.cend(); i++) {
        total = total + (*i * (Interval<double>(1) - *i));
    }
    return total;
}

Interval<double> impurityAddition(const BinarySamples &counts, int n) {
    Interval<double> p = estimateBernoulliAddition(counts, n);
    return p * (Interval<double>(1) - p) * Interval<double>(2);
}

Interval<double> jointImpurityAddition(const std::vector<int> &counts1, int n1, const std::vector<int> &counts2, int n2) {
    int total1 = accumulate(counts1.cbegin(), counts1.cend(), 0);
    int total2 = accumulate(counts2.cbegin(), counts2.cend(), 0);

    // Size 1 and Size 2 represent total size of the dataset - these won't change.
    Interval<double> size1(total1, total1);
    Interval<double> size2(total2, total2);
    return size1 * impurityAddition(counts1, n1) + size2 * impurityAddition(counts2, n2);
}

// ----------------------------------------------------------------------------------------
// ONE-SIDED DATA ADDITION
// ----------------------------------------------------------------------------------------
Interval<double> estimateBernoulliAdditionLop(const BinarySamples &counts, int n) {
    if (counts.num_zeros + counts.num_ones <= n) {
        return Interval<double>(0,1);
    }

    BinarySamples minimizer,maximizer;
    // Under this model, we cannot create any new zero's and cannot decrease the number of either
    minimizer.num_zeros = counts.num_zeros;
    minimizer.num_ones = counts.num_ones;
    maximizer.num_zeros = counts.num_zeros;
    maximizer.num_ones = counts.num_ones + n;
    return Interval<double>(estimateBernoulli(minimizer), estimateBernoulli(maximizer));
}

CategoricalDistribution<Interval<double>> estimateCategoricalAdditionLop(const std::vector<int> &counts, int n) {
    // lopsided label flipping
    int count_total = accumulate(counts.cbegin(), counts.cend(), 0);

    CategoricalDistribution<Interval<double>> ret(counts.size());
    for (unsigned int i = 0; i < ret.size(); i++) {
        int count_not_y = count_total - counts[i];
        BinarySamples minimizer,maximizer;
        maximizer.num_ones = counts[i] + n;
        maximizer.num_zeros = count_not_y;
        minimizer.num_ones = counts[i];
        minimizer.num_zeros = count_not_y;
        ret[i] = Interval<double>(estimateBernoulli(minimizer), estimateBernoulli(maximizer));
    }

    return ret;
}

Interval<double> impurityAdditionLop(const std::vector<int> &counts, int n) {
   
    CategoricalDistribution<Interval<double>> p = estimateCategoricalAdditionLop(counts, n);
    Interval<double> total(0);
    for (auto i= p.cbegin(); i != p.cend(); i++) {
        total = total + (*i * (Interval<double>(1) - *i));
    }
    return total;
}

Interval<double> impurityAdditionLop(const BinarySamples &counts, int n) {
    // For one-sided data addition
    Interval<double> p = estimateBernoulliAdditionLop(counts, n);
    return p * (Interval<double>(1) - p) * Interval<double>(2);
}

Interval<double> jointImpurityAdditionLop(const std::vector<int> &counts1, int n1, const std::vector<int> &counts2, int n2) {
    int total1 = accumulate(counts1.cbegin(), counts1.cend(), 0);
    int total2 = accumulate(counts2.cbegin(), counts2.cend(), 0);
        
    Interval<double> size1(total1, total1);
    Interval<double> size2(total2, total2);

    return size1 * impurityAdditionLop(counts1, total1) + size2 * impurityAdditionLop(counts2, total2);
}

