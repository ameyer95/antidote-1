#ifndef MNISTEXPERIMENT_H
#define MNISTEXPERIMENT_H

#include "CategoricalDistribution.hpp"
#include "DataSet.hpp"
#include "ExperimentDataWrangler.h"
#include "Interval.h"
#include <string>
#include <vector>


class MNISTExperiment {
private:
    const DataSet *mnist_training;
    const DataSet *mnist_test;

    ExperimentDataWrangler *wrangler;

public:
    MNISTExperiment(ExperimentDataWrangler *wrangler);

    int test_size() { return mnist_test->rows.size(); }
    int groundTruth(int test_index) const { return mnist_test->rows[test_index].y; }
    CategoricalDistribution<double> run_concrete(int depth, int test_index);
    CategoricalDistribution<Interval<double>> run_abstract(int depth, int test_index, int num_dropout);
    CategoricalDistribution<Interval<double>> run_abstract_disjuncts(int depth, int test_index, int num_dropout);
    CategoricalDistribution<Interval<double>> run_abstract_bounded_disjuncts(int depth, int test_index, int num_dropout, int disjunct_bound, const std::string &merge_mode);
};


#endif
