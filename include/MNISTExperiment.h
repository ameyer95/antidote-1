#ifndef MNISTEXPERIMENT_H
#define MNISTEXPERIMENT_H

#include "AbstractSemantics.h"
#include "ASTNode.h"
#include "ConcreteSemantics.h"
#include "Interval.h"
#include "SimplestBoxInstantiation.h"
#include <string>
#include <vector>


class MNISTExperiment {
private:
    const std::vector<DataRow> *mnist_training;
    const std::vector<DataRow> *mnist_test;
    const std::vector<BitVectorPredicate> *predicates;

    void loadMNIST(std::string mnistPrefix);
    std::vector<BitVectorPredicate>* createPredicates();

public:
    MNISTExperiment(std::string mnistPrefix);
    ~MNISTExperiment();

    int test_size() { return mnist_test->size(); }
    double run_concrete(int depth, int test_index);
    Interval<double> run_abstract(int depth, int test_index, int num_dropout);
};


#endif
