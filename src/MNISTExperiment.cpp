#include "MNISTExperiment.h"
#include "ASTNode.h"
#include "ConcreteSemantics.h"
#include "MNIST.h"
#include <string>
#include <utility>
#include <iostream>
using namespace std;

MNISTExperiment::MNISTExperiment(string mnistPrefix) {
    predicates = new vector<BitVectorPredicate>();
    for(int i = 0; i < MNIST_IMAGE_SCALE * MNIST_IMAGE_SCALE; i++) {
        predicates->push_back(BitVectorPredicate(i));
    }
    loadMNIST(mnistPrefix);
}

void MNISTExperiment::loadMNIST(string mnistPrefix) {
    pair<BooleanDataSet*, BooleanDataSet*> mnist;//XXX
    training = mnist.first;
    test = mnist.second;
    //cout << "training set size: " << training->size() << " summary: " << training->summary() << endl;
    //cout << "test set size: " << test->size() << " summary: " << test->summary() << endl;
}


double MNISTExperiment::run(int depth, int test_index) {
    ASTNode* program = ASTNode::buildTree(depth);
    ConcreteSemantics sem;
    return sem.execute(test->getRow(test_index).first, training, predicates, program);
}