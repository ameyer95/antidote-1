#ifndef EXPERIMENTBACKEND_H
#define EXPERIMENTBACKEND_H

#include "CategoricalDistribution.hpp"
#include "CommonEnums.h"
#include "DataSet.hpp"
#include "Interval.h"
#include <map>
#include <set>


class ExperimentBackend {
private:
    const DataSet *training;
    const DataSet *test;

public:
    template <typename T>
    struct Result {
        CategoricalDistribution<T> posterior;
        std::set<int> possible_classifications;
        int ground_truth;
    };
   // bool use_label_flipping;

    ExperimentBackend(const DataSet *training, const DataSet *test);

    int test_size() { return test->rows.size(); }
    int groundTruth(int test_index) const { return test->rows[test_index].y; }

    Result<double> run_concrete(int depth, int test_index);
    Result<Interval<double>> run_abstract(int depth, int test_index, int num_dropout, int num_add, std::pair<int, int> add_sens_info, int num_labels_flip, std::pair<int, int> label_sens_info, int num_features_flip, int feature_flip_index, float feature_flip_amt);
    Result<Interval<double>> run_abstract_disjuncts(int depth, int test_index, int num_dropout, int num_add, std::pair<int, int> add_sens_info, int num_labels_flip, std::pair<int, int> label_sens_info, int num_features_flip, int feature_flip_index, float feature_flip_amt);
    Result<Interval<double>> run_abstract_bounded_disjuncts(int depth, int test_index, int num_dropout, int num_add, std::pair<int, int> add_sens_info, int num_labels_flip, std::pair<int, int> label_sens_info, int num_features_flip, int feature_flip_index, float feature_flip_amt, int disjunct_bound, const DisjunctsMergeMode &merge_mode);

    std::map<int,int> run_test(int depth, int test_index, int num_dropout, int num_trials, unsigned int seed);
};


#endif
