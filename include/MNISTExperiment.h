#ifndef MNISTEXPERIMENT_H
#define MNISTEXPERIMENT_H

#include "ASTNode.h"
#include "ConcreteSemantics.h"
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
    double run(int depth, int test_index);
};


#endif
