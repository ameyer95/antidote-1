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
    loadMNIST(mnistPrefix);
    predicates = createPredicates();
}

vector<BitVectorPredicate>* MNISTExperiment::createPredicates() {
    vector<BitVectorPredicate> *ret = new vector<BitVectorPredicate>();
    ret->reserve(MNIST_IMAGE_SIZE);
    for(int i = 0; i < MNIST_IMAGE_SIZE; i++) {
        ret->push_back(BitVectorPredicate(i));
    }
    return ret;
}

void MNISTExperiment::loadMNIST(string mnistPrefix) {
    RawMNIST raw_mnist_training(MNISTMode::TRAINING, mnistPrefix);
    RawMNIST raw_mnist_test(MNISTMode::TEST, mnistPrefix);
    mnist_training = simplified(raw_mnist_training);
    mnist_test = simplified(raw_mnist_test);
}

double MNISTExperiment::run(int depth, int test_index) {
    ASTNode* program = ASTNode::buildTree(depth);
    ConcreteSemantics sem;
    BooleanDataSet *training_set = new BooleanDataSet(new DataReferences<DataRow>(mnist_training));
    double ret = sem.execute((*mnist_test)[test_index].first, training_set, predicates, program);
    delete training_set;
    delete program;
    return ret;
}
