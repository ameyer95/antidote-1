#include "MNISTExperiment.h"
#include "ASTNode.h"
#include "ConcreteSemantics.h"
#include "MNIST.h"
#include <string>
#include <utility>
#include <vector>
using namespace std;

// Uses 128 as the binary threshold (each pixel is a byte)
inline Input Image_to_Input(const Image &image) {
    vector<bool> ret(MNIST_IMAGE_SIZE);
    for(int i = 0; i < MNIST_IMAGE_SIZE; i++) {
        ret[i] = *(image + i) > 128;
    }
    return ret;
}

// The classes pair assigns the boolean labels first -> 0 and second -> 1
vector<DataRow>* simplified(const RawMNIST &mnist, pair<int, int> classes = make_pair(1, 7)) {
    vector<DataRow> *ret = new vector<DataRow>();
    for(int i = 0; i < mnist.size(); i++) {
        if(mnist[i].second == classes.first || mnist[i].second == classes.second) {
            ret->push_back(make_pair(Image_to_Input(mnist[i].first), mnist[i].second == classes.second));
        }
    }
    return ret;
}

MNISTExperiment::MNISTExperiment(string mnistPrefix) {
    predicates = new vector<BitVectorPredicate>();
    for(int i = 0; i < MNIST_IMAGE_SIZE; i++) {
        predicates->push_back(BitVectorPredicate(i));
    }
    loadMNIST(mnistPrefix);
}

void MNISTExperiment::loadMNIST(string mnistPrefix) {
    RawMNIST mnist_training(MNISTMode::TRAINING, mnistPrefix);
    RawMNIST mnist_test(MNISTMode::TEST, mnistPrefix);
    training = new BooleanDataSet(new DataReferences<DataRow>(simplified(mnist_training)));
    test = new DataReferences<DataRow>(simplified(mnist_test));
}

double MNISTExperiment::run(int depth, int test_index) {
    ASTNode* program = ASTNode::buildTree(depth);
    ConcreteSemantics sem;
    return sem.execute((*test)[test_index].first, training, predicates, program);
}
