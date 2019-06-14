#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include "MNISTReader.h"
#include "DataSet.h"
#include "ConcreteSemantics.h"
#include "ASTNode.h"
#include <string>
using namespace std;


class Experiment {
private:
    DataSet *training, *test;
    PredicateSet *predicates;

    void loadMNIST(string mnistPrefix);

public:
    Experiment(string mnistPrefix);
    double run(int depth, int test_index);
};


#endif
