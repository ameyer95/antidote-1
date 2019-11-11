#ifndef EXPERIMENTBACKEND_H
#define EXPERIMENTBACKEND_H

#include "CategoricalDistribution.hpp"
#include "CommonEnums.h"
#include "DataSet.hpp"
#include "Interval.h"
#include <string>
#include <vector>


class ExperimentBackend {
private:
    const DataSet *training;
    const DataSet *test;

public:
    ExperimentBackend(const DataSet *training, const DataSet *test);

    int test_size() { return test->rows.size(); }
    int groundTruth(int test_index) const { return test->rows[test_index].y; }
    CategoricalDistribution<double> run_concrete(int depth, int test_index);
    CategoricalDistribution<Interval<double>> run_abstract(int depth, int test_index, int num_dropout);
    CategoricalDistribution<Interval<double>> run_abstract_disjuncts(int depth, int test_index, int num_dropout);
    CategoricalDistribution<Interval<double>> run_abstract_bounded_disjuncts(int depth, int test_index, int num_dropout, int disjunct_bound, const DisjunctsMergeMode &merge_mode);
};


#endif
