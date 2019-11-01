#ifndef DATASET_HPP
#define DATASET_HPP

/**
 * In general, we imagine that a training (or test) set T
 * has rows consisting of elements in some space X x Y.
 * This file defines the general object for representing such a dataset.
 */

#include "Feature.hpp"
#include "CategoricalDistribution.h"
#include <vector>

// For now, X is always a FeatureVector and Y is always an int.
struct DataRow {
    FeatureVector x;
    int y;
};

struct DataSet {
    FeatureVectorHeader feature_types; // Data about the X columns
    int num_categories; // Size of Y
    std::vector<DataRow> rows;
};

#endif
