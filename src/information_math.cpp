#include "information_math.h"
#include "CategoricalDistribution.hpp"
#include "Interval.h"
#include <algorithm> // for std::max/min
#include <numeric> // for std::accumulate (easy summations)
#include <utility>
#include <vector>
#include <iostream>
using namespace std;

double estimateBernoulli0(const BinarySamples &counts) {
    return (double)counts.num_zeros / (counts.num_zeros + counts.num_ones);
}
double estimateBernoulli(const BinarySamples &counts) {
    return (double)counts.num_ones / (counts.num_zeros + counts.num_ones);
}

Interval<double> estimateBernoulli(const BinarySamples &counts, int num_dropout, int num_add, int num_labels_flip, int num_features_flip, std::pair<int, int> label_sens_info, std::pair<int, int> add_sens_info) {
    // When num_dropout >= num_zeros + num_ones, anything is possible.
    // In the == case, this is because we assume estimating from an empty set is undefined behavior.
    if(counts.num_zeros + counts.num_ones <= num_dropout + num_labels_flip) {
        return Interval<double>(0, 1);
    }

    std::cout << "I AM HERE" << std::endl;

    // Since this is effectively computing an average of a collection of 0s and 1s,
    // extremal behavior occurs either when maximally many 1s are removed
    // or when maximally many 0s are removed.
    // (This is more precise than the obvious count-interval division)
    BinarySamples minimizer, maximizer;
    // NOTE: right now this is hard-coded for compas one sided (can only flip labels from 1 to 0)
    maximizer.num_ones = min(counts.num_ones + num_labels_flip + num_add, counts.num_zeros + counts.num_ones + num_add);
    maximizer.num_zeros = max(0, counts.num_zeros - num_dropout - num_labels_flip);
    
    if (label_sens_info.first > -1) {
        minimizer.num_ones = max(0, counts.num_ones - num_dropout);
        if (add_sens_info.first > -1) {
            minimizer.num_zeros = counts.num_zeros; //COMPAS: can't increase # of zeros
        } else {
            minimizer.num_zeros = counts.num_zeros + num_add; // can't flip any labels from 0 to 1
        }
    } else if (add_sens_info.first > -1) {
        minimizer.num_ones = max(0, counts.num_ones - num_dropout - num_labels_flip);
        minimizer.num_zeros = min(counts.num_zeros + num_labels_flip, counts.num_zeros + counts.num_ones); // Can't add label 0
    } else {
        minimizer.num_ones = max(0, counts.num_ones - num_dropout - num_labels_flip);
        minimizer.num_zeros = min(counts.num_zeros + num_labels_flip + num_add, counts.num_zeros + counts.num_ones + num_add);
    }
    return Interval<double>(estimateBernoulli(minimizer), estimateBernoulli(maximizer));
}

double impurity(const BinarySamples &counts) {
    double p = estimateBernoulli(counts);
    return p * (1-p) / 2; // Gini impurity
}

Interval<double> impurity(const BinarySamples &counts, int num_dropout, int num_add, int num_labels_flip, int num_features_flip, std::pair<int, int> label_sens_info, std::pair<int, int> add_sens_info) {
    Interval<double> p = estimateBernoulli(counts, num_dropout, num_add, num_labels_flip, num_features_flip, label_sens_info, add_sens_info);
    return p * (Interval<double>(1) - p) * Interval<double>(2);
}

double jointImpurity(const BinarySamples &counts1, const BinarySamples &counts2) {
    return (counts1.num_zeros + counts1.num_ones) * impurity(counts1)
        + (counts2.num_zeros + counts2.num_ones) * impurity(counts2);
}

Interval<double> jointImpurity(const BinarySamples &counts1, int num_dropout1, int num_add1, int num_labels_flip1, int num_features_flip1,
                             const BinarySamples &counts2, int num_dropout2, int num_add2, int num_labels_flip2, int num_features_flip2, 
                             std::pair<int, int> label_sens_info, std::pair<int, int> add_sens_info) {
    int total1 = counts1.num_zeros + counts1.num_ones;
    int total2 = counts2.num_zeros + counts2.num_ones;
    // TO DO ANNA ONE-SIDED THIS IS WRONG (or at least, not precise)
    Interval<double> size1(total1 - num_dropout1, total1 + num_add1);
    Interval<double> size2(total2 - num_dropout2, total2 + num_add2);
    return size1 * impurity(counts1, num_dropout1, num_add1, num_labels_flip1, num_features_flip2, label_sens_info, add_sens_info) + 
            size2 * impurity(counts2, num_dropout2, num_add2, num_labels_flip2, num_features_flip2, label_sens_info, add_sens_info);
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

CategoricalDistribution<Interval<double>> estimateCategorical(const std::vector<int> &counts, int num_dropout, int num_add, int num_labels_flip, int num_features_flip, std::pair<int, int> label_sens_info, std::pair<int, int> add_sens_info) {
    int count_total = accumulate(counts.cbegin(), counts.cend(), 0);
    // When num_dropout >= count_total, anything is possible.
    // In the == case, this is because we assume estimating from an empty set is undefined behavior.
    if(count_total <= num_dropout + num_labels_flip) {
        return CategoricalDistribution<Interval<double>>(counts.size(), Interval<double>(0,1));
    }

    CategoricalDistribution<Interval<double>> ret(counts.size());
    // We estimate each component individually.
    // Since this is effectively computing an average of a collection of 0s and 1s,
    // extremal behavior occurs either when maximally many 1s are removed
    // or when maximally many 0s are removed.
    // (This is more precise than the obvious count-interval division)    
    
    int count0 = counts[0];
    int count1 = counts[1];
    BinarySamples minimizer, maximizer;
    //maximizer.num_ones = min(count1 + num_labels_flip + num_add, count0 + count1 + num_add); // AI can flip from 0 to 1
    //maximizer.num_zeros = max(0, count0 - num_dropout - num_labels_flip); // AI can flip from 0 to 1
    minimizer.num_ones = max(0, count1 - num_dropout - num_labels_flip); // COMPAS can flip from 1 to 0
    minimizer.num_zeros = min(count0 + num_labels_flip + num_add, count0 + count1 + num_add); // COMPAS can flip from 1 to 0

    if (label_sens_info.first > -1) {
       // minimizer.num_ones = max(0, count1 - num_dropout); // AI can't decrease num ones via label-flipping
        maximizer.num_ones = min(count1 + num_add, count0 + count1 + num_add); // COMPAS can't increase num ones
        if (add_sens_info.first > -1) {
            // This needs to be tailored to specific adding rules, right now, not set up for anything in particular
            minimizer.num_zeros = count0; // can't increase # of zeros
        } else {
            // minimizer.num_zeros = count0 + num_add; // AI: can't flip any labels from 1 to 0
            maximizer.num_zeros = max(0, count0 - num_dropout); // COMPAS can't decrease 0's by flipping any to 1's
        }
    } else if (add_sens_info.first > -1) {
        minimizer.num_ones = max(0, count1 - num_dropout - num_labels_flip);
        minimizer.num_zeros = min(count0 + num_labels_flip, count_total); // Can't add label 0
    } else {
        minimizer.num_ones = max(0, count1 - num_dropout - num_labels_flip);
        minimizer.num_zeros = min(count0 + num_labels_flip + num_add, count0 + count1 + num_add);
        maximizer.num_ones = min(count1 + num_labels_flip + num_add, count0 + count1 + num_add);
        maximizer.num_zeros = max(0, count0 - num_dropout - num_labels_flip); 
    }

    // question - flip role of min and max in 0 line to have [smaller, larger]?
    ret[0] = Interval<double>(estimateBernoulli0(maximizer), estimateBernoulli0(minimizer));
    ret[1] = Interval<double>(estimateBernoulli(minimizer), estimateBernoulli(maximizer));

    if (ret[0].get_upper_bound() < ret[0].get_lower_bound()) {
        std::cout << "PROBLEM INTERVAL IS BACKWARDS (0)" << std::endl;
        std::cout << "count0: " << std::to_string(count0) << ", count1: " << std::to_string(count1) << std::endl;
        std::cout << "max.num0: " << std::to_string(maximizer.num_zeros) << ", max.num1: " << std::to_string(maximizer.num_ones) << ", min.num0: " << std::to_string(minimizer.num_zeros) << ", min.num1: " << std::to_string(minimizer.num_ones) << std::endl;
    }
    if (ret[1].get_upper_bound() < ret[1].get_lower_bound()) {
        std::cout << "PROBLEM INTERVAL IS BACKWARDS (1)" << std::endl;
        std::cout << "count0: " << std::to_string(count0) << ", count1: " << std::to_string(count1) << std::endl;

    }
    
    return ret; 
}

Interval<double> impurity(const std::vector<int> &counts, int num_dropout, int num_add, int num_labels_flip, int num_features_flip, std::pair<int, int> label_sens_info, std::pair<int, int> add_sens_info) {
    // TODO can be more precise
    CategoricalDistribution<Interval<double>> p = estimateCategorical(counts, num_dropout, num_add, num_labels_flip, num_features_flip, label_sens_info, add_sens_info);
    Interval<double> total(0);
    for(auto i = p.cbegin(); i != p.cend(); i++) {
        total = total + (*i * (Interval<double>(1) - *i));
    }
    return total;
}

Interval<double> jointImpurity(const std::vector<int> &counts1, int num_dropout1, int num_add1, int num_labels_flip1, int num_features_flip1,
                              const std::vector<int> &counts2, int num_dropout2, int num_add2, int num_labels_flip2, int num_features_flip2, 
                              std::pair<int, int> label_sens_info, std::pair<int, int> add_sens_info) {
    // In the one-sided case, num_labels_flip is accurate
    int total1 = accumulate(counts1.cbegin(), counts1.cend(), 0);
    int total2 = accumulate(counts2.cbegin(), counts2.cend(), 0);

    // TO DO ANNA more nuanced for one-sided
    Interval<double> size1(total1 - num_dropout1, total1 + num_add1);
    Interval<double> size2(total2 - num_dropout2, total2 + num_add2);
    Interval<double> imp = impurity(counts1, num_dropout1, num_add1, num_labels_flip1, num_features_flip2, label_sens_info, add_sens_info);
    if (imp.get_upper_bound() < imp.get_lower_bound()) {
        std::cout << "impurity.lower: " << std::to_string(imp.get_lower_bound()) << ", impurity.upper: " << std::to_string(imp.get_upper_bound()) << std::endl;
    }
    return size1 * impurity(counts1, num_dropout1, num_add1, num_labels_flip1, num_features_flip2, label_sens_info, add_sens_info) + 
            size2 * impurity(counts2, num_dropout2, num_add2, num_labels_flip2, num_features_flip2, label_sens_info, add_sens_info);
}