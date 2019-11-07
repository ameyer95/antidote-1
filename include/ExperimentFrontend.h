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
        std::string dataset; // Should be "mnist" or "iris" or ...
        bool use_abstract; // When false, use concrete semantics
        bool with_disjuncts; // When true, use_abstract must also be true, and this says to do the more precise domain
        std::vector<int> num_dropouts; // For when use_abstract == true
        std::optional<int> disjunct_bound; // Optionally, has_value only when with_disjuncts is true
        std::string merge_mode; // For when disjunct_bound.has_value(), takes value "greedy" or "optimal"
    } params;

    // TODO access this information from some more-central location
    const std::set<std::string> dataset_options = {"mnist", "iris", "cancer", "wine"};
    const std::set<std::string> merge_options = {"greedy", "optimal"};

    ArgParse p;
    MNISTExperiment *e;
    ExperimentDataWrangler *wrangler;

    void createCommandLineArguments();
    void performSingleTest(int depth, int test_index);

public:
    ExperimentFrontend();

    bool processCommandLineArguments(int argc, char ** const &argv);
    void performExperiments();
};


#endif
