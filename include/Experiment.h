#ifndef EXPERIMENT_H
#define EXPERIMENT_H

#include "ASTNode.h"
#include "ConcreteSemantics.h"
#include <string>
#include <vector>
using namespace std;


class Experiment {
private:
    BooleanDataSet *training, *test;
    vector<BitVectorPredicate> *predicates;

    void loadMNIST(string mnistPrefix);

public:
    Experiment(string mnistPrefix);
    double run(int depth, int test_index);
};


#endif
