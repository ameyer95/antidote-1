#ifndef MNISTEXPERIMENT_H
#define MNISTEXPERIMENT_H

#include "ASTNode.h"
#include "ConcreteSemantics.h"
#include <string>
#include <vector>
using namespace std;


class MNISTExperiment {
private:
    BooleanDataSet *training;
    DataReferences<DataRow> *test;
    vector<BitVectorPredicate> *predicates;

    void loadMNIST(string mnistPrefix);

public:
    MNISTExperiment(string mnistPrefix);
    double run(int depth, int test_index);
};


#endif
