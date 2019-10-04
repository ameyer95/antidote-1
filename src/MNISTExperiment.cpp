#include "MNISTExperiment.h"
#include "AbstractSemantics.h"
#include "ASTNode.h"
#include "ConcreteSemantics.h"
#include "Interval.h"
#include "MNIST.h"
#include "SimplestBoxInstantiation.h"
#include <string>
#include <utility>
#include <vector>
using namespace std;

// Uses 128 as the binary threshold (each pixel is a byte)
inline vector<bool> Image_to_Input(const Image &image) {
    vector<bool> ret(MNIST_IMAGE_SIZE);
    for(int i = 0; i < MNIST_IMAGE_SIZE; i++) {
        ret[i] = *(image + i) > 128;
    }
    return ret;
}

// The classes pair assigns the boolean labels first -> 0 and second -> 1
vector<BooleanXYPair>* simplified(const RawMNIST &mnist, pair<int, int> classes = make_pair(1, 7)) {
    vector<BooleanXYPair> *ret = new vector<BooleanXYPair>();
    for(unsigned int i = 0; i < mnist.size(); i++) {
        if(mnist[i].second == classes.first || mnist[i].second == classes.second) {
            ret->push_back(make_pair(Image_to_Input(mnist[i].first), mnist[i].second == classes.second));
        }
    }
    return ret;
}

MNISTExperiment::MNISTExperiment(string mnistPrefix) {
    // The following allocate and populate this->{mnist_training, mnist_test, predicates}
    // (Accordingly, that's what the destructor cleans up)
    loadMNIST(mnistPrefix);
    predicates = createPredicates();
}

MNISTExperiment::~MNISTExperiment() {
    delete mnist_training;
    delete mnist_test;
    delete predicates;
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

double MNISTExperiment::run_concrete(int depth, int test_index) {
    ProgramNode* program = buildTree(depth);
    // By putting all of the following on the stack, we don't have to do heap deallocation
    ConcreteSemantics sem;
    DataReferences<BooleanXYPair> training_references(mnist_training);
    BooleanDataSet training_dataset(&training_references);
    double ret = sem.execute((*mnist_test)[test_index].first, &training_dataset, predicates, program);
    delete program;
    return ret;
}

Interval<double> MNISTExperiment::run_abstract(int depth, int test_index, int num_dropout) {
    ProgramNode* program = buildTree(depth);
    vector<bool> test_input = (*mnist_test)[test_index].first;
    SimplestBoxDomain box_domain(test_input); // XXX there is probably a more proper way to pass around this information, especially since the predicatedomain's top element computation then uses test_input.size()
    AbstractSemantics<SimplestBoxDomain, SimplestBoxAbstraction, vector<bool>> sem(&box_domain);
    DataReferences<BooleanXYPair> training_references(mnist_training);
    SimplestBoxAbstraction initial_state(BooleanDropoutSet(training_references, num_dropout),
                                         BitvectorPredicateAbstraction(),
                                         Interval<double>());
    SimplestBoxAbstraction ret = sem.execute(test_input, initial_state, program);
    delete program;
    return ret.posterior_distribution_abstraction;
}
