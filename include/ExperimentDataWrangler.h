#ifndef EXPERIMENTDATAWRANGLER_H
#define EXPERIMENTDATAWRANGLER_H

#include "DataSet.hpp"
#include "MNIST.h"
#include "UCI.h"
#include <map>
#include <string>
#include <utility>

/**
 * Since we have datasets from different kinds of sources,
 * this is where we unify them into a standard dataset type for experiments.
 * ExperimentDataWrangler's design includes:
 *     - training/test set division
 *     - conversion between CategoricalDistribution int indices and string names
 *     - loads from any of the different dataset sources
 */

// Experiment code can request to fetch a dataset from options in the following enum.
enum class ExperimentDataEnum {
    MNIST_BOOLEAN_1_7,
    MNIST,
    UCI_IRIS,
    UCI_CANCER,
    UCI_WINE,
};

// Fetches return this structure. Note that they still need a DataReferences wrapper.
struct ExperimentData {
    DataSet *training;
    DataSet *test;
    std::vector<std::string> class_labels;
};


class ExperimentDataWrangler {
private:
    std::map<ExperimentDataEnum, const ExperimentData*> cache;
    std::string path_prefix;

    void loadData(const ExperimentDataEnum &dataset);
    ExperimentData* loadSimplifiedMNIST(const std::pair<int, int> &classes);

public:
    ExperimentDataWrangler(const std::string &path_prefix);
    ~ExperimentDataWrangler(); // When destructed, deallocates all the fetch()'d data

    const ExperimentData* fetch(const ExperimentDataEnum &dataset);
};


#endif
