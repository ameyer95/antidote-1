#ifndef MNISTEXPERIMENT_H
#define MNISTEXPERIMENT_H

#include "ASTNode.h"
#include "ConcreteSemantics.h"
#include "DropoutSemantics.h"
#include "Interval.h"
#include <string>
#include <vector>
using namespace std;


class MNISTExperiment {
private:
    const vector<DataRow> *mnist_training;
    const vector<DataRow> *mnist_test;
    const vector<BitVectorPredicate> *predicates;

    void loadMNIST(string mnistPrefix);
    vector<BitVectorPredicate>* createPredicates();

public:
    MNISTExperiment(string mnistPrefix);
    ~MNISTExperiment();

    int test_size() { return mnist_test->size(); }
    double run_concrete(int depth, int test_index);
    Interval<double> run_abstract(int depth, int test_index, int num_dropout);
};


#endif
