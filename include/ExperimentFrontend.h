#ifndef EXPERIMENTFRONTEND_H
#define EXPERIMENTFRONTEND_H

#include "ArgParse.h"
#include "ExperimentBackend.h"
#include "ExperimentDataWrangler.h"
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
        std::string arff_train; 
        std::string arff_test; 
        int arff_label_ind;
        bool use_bin; 
        float bin_thres; 
        bool use_abstract; // When false, use concrete semantics
        bool with_disjuncts; // When true, use_abstract must also be true, and this says to do the more precise domain
        bool label_flipping; // When true, use the label flipping data poisoning model
        bool data_addition; // When true, use data addition data poisoning model
        int sensitive_feature; // Index in the dataset of the sensitive feature (e.g., race or gender)
        // TO DO ANNA - extend protected_value to be an array
        float protected_value; // Integer of protected value for the sensitive feature
        std::vector<int> num_dropouts; // For when use_abstract == true
        std::optional<int> disjunct_bound; // Optionally, has_value only when with_disjuncts is true
        DisjunctsMergeMode merge_mode; // For when disjunct_bound.has_value()
        struct RandomTest {
            bool flag; // Whether to do a random test
            int num_dropout;
            int num_trials;
            unsigned int seed;
        } random_test;
    } params;

    bool verbose;
    ArgParse p;
    ExperimentBackend *e;
    ExperimentDataWrangler *wrangler;
    const ExperimentData *current_data; // the wrangler handles this deallocation

    void createCommandLineArguments();
    void performSingleTest(int depth, int test_index);
    void performAbstractTests(int depth, int test_index);

    std::string output_to_json(int depth, int test_index, const ExperimentBackend::Result<double> &result);
    std::string output_to_json(int depth, int test_index, const ExperimentBackend::Result<Interval<double>> &result);
    std::string output_to_json(int depth, int test_index, const std::map<int,int> &result);

    void output(const std::string &message, bool force=false);

public:
    ExperimentFrontend();

    bool processCommandLineArguments(int argc, char ** const &argv);
    void performExperiments();
};


#endif
