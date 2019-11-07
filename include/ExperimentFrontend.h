#ifndef EXPERIMENTFRONTEND_H
#define EXPERIMENTFRONTEND_H

#include "ArgParse.h"
#include "ExperimentDataWrangler.h"
#include "MNISTExperiment.h"
#include <optional>
#include <set>
#include <string>
#include <vector>

/**
 * This file/class is responsible for the following:
 *     - creating and parsing arguments
 *     - calling the actual semantics running class
 *     - outputting relevant results
 */


class ExperimentFrontend {
private:
    struct RunParams {
        std::vector<int> depths;
        bool test_all; // When false, use only the indices in test_indices
        std::vector<int> test_indices;
        std::string data_prefix;
        ExperimentDataEnum dataset;
        bool use_abstract; // When false, use concrete semantics
        bool with_disjuncts; // When true, use_abstract must also be true, and this says to do the more precise domain
        std::vector<int> num_dropouts; // For when use_abstract == true
        std::optional<int> disjunct_bound; // Optionally, has_value only when with_disjuncts is true
        DisjunctsMergeMode merge_mode; // For when disjunct_bound.has_value()
    } params;

    ArgParse p;
    MNISTExperiment *e;
    ExperimentDataWrangler *wrangler;
    const ExperimentData *current_data; // the wrangler handles this deallocation

    void createCommandLineArguments();
    void performSingleTest(int depth, int test_index);

public:
    ExperimentFrontend();

    bool processCommandLineArguments(int argc, char ** const &argv);
    void performExperiments();
};


#endif
