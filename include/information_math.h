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

double estimateBernoulli0(const BinarySamples &counts);
double estimateBernoulli(const BinarySamples &counts);
Interval<double> estimateBernoulli(const BinarySamples &counts, int num_dropout, int num_add, int num_labels_flip, int num_features_flip,
                                    std::pair<int, int> label_sens_info, std::pair<int, int> add_sens_info);

// Computes the Gini Impurity
double impurity(const BinarySamples &counts);
double impurityLabels(const BinarySamples &counts);
Interval<double> impurity(const BinarySamples &counts, int num_dropout, int num_add, int num_labels_flip, int num_features_flip,
                            std::pair<int, int> label_sens_info, std::pair<int, int> add_sens_info);

// These are non-normalized
double jointImpurity(const BinarySamples &counts1, const BinarySamples &counts2);
Interval<double> jointImpurity(const BinarySamples &counts1, int num_dropout1, int num_add1, int num_labels_flip1, int num_features_flip1,
                               const BinarySamples &counts2, int num_dropout2, int num_add2, int num_labels_flip2, int num_features_flip2,
                               std::pair<int, int> label_sens_info, std::pair<int, int> add_sens_info);

CategoricalDistribution<double> estimateCategorical(const std::vector<int> &counts);
double impurity(const std::vector<int> &counts);
double jointImpurity(const std::vector<int> &counts1, const std::vector<int> &counts2);

CategoricalDistribution<Interval<double>> estimateCategorical(const std::vector<int> &counts, int num_dropout, int num_add, 
                                                int num_labels_flip, int num_features_flip, 
                                                std::pair<int, int> label_sens_info, std::pair<int, int> add_sens_info);
Interval<double> impurity(const std::vector<int> &counts, int num_dropout, int num_add, int num_labels_flip, int num_features_flip,
                        std::pair<int, int> label_sens_info, std::pair<int, int> add_sens_info);
Interval<double> jointImpurity(const std::vector<int> &counts1, int num_dropout1, int num_add1, int num_labels_flip1, int num_features_flip1,
                               const std::vector<int> &counts2, int num_dropout2, int num_add2, int num_labels_flip2, int num_features_flip2,
                               std::pair<int, int> label_sens_info, std::pair<int, int> add_sens_info);


#endif
