#include "Experiment.h"
#include "MNISTReader.h"
#include "DataSet.h"
#include "ASTNode.h"
#include <string>
#include <utility>
#include <iostream>
using namespace std;

Experiment::Experiment(string mnistPrefix) {
    vector<Predicate> *phis = new vector<Predicate>();
    loadMNIST(mnistPrefix);
    for(int i = 0; i < training->getRow(0).first.size(); i++) {
        phis->push_back(Predicate(i));
    }
    predicates = new PredicateSet(phis);
}

void Experiment::loadMNIST(string mnistPrefix) {
    pair<DataSet*, DataSet*> mnist;//XXX = MNIST_readAsDataSet(mnistPrefix);
    training = mnist.first;
    test = mnist.second;
    cout << "training set size: " << training->size() << " summary: " << training->summary() << endl;
    cout << "test set size: " << test->size() << " summary: " << test->summary() << endl;
}


double Experiment::run(int depth, int test_index) {
    ASTNode* program = ASTNode::buildTree(depth);
    ConcreteSemantics sem;
    return sem.execute(test->getRow(test_index).first, training, predicates, program);
}
