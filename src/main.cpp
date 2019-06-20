#include "ASTNode.h"
#include "MNISTExperiment.h"
#include "MNIST.h"
#include "PrettyPrinter.h"
#include <iostream>
#include <utility>
#include <cstdint>
using namespace std;


void test_build_ast(int depth) {
    ASTNode *root = ASTNode::buildTree(depth);
    PrettyPrinter p = PrettyPrinter();
    cout << "Depth " << depth << endl;
    root->accept(p);
    cout << p.getString() << endl;
}

void test_build_asts() {
    for(int i = 0; i < 3; i++) {
        test_build_ast(i);
    }
}

void print_digit(uint8_t *start) {
    for(int row = 0; row < 28; row++) {
        for(int col = 0; col < 28; col++) {
            uint8_t pixel = *(start + 28*row + col);
            cout << (pixel < 128 ? " " : "0");
        }
        cout << endl;
    }
}

void test_load_MNIST() {
    pair<LabelFile, ImageFile> training, test;
    training = MNIST_readTrainingSet("data/");
    test = MNIST_readTestSet("data/");
    for(int i = 0; i < 10; i++) {
        cout << "This is supposedly a " << (int)training.first.labels[i] << endl;
        print_digit(training.second.pixels + i*28*28*sizeof(uint8_t));
        cout << "==================================================" << endl << endl;
    }
}

void test_MNIST() {
    MNISTExperiment e("data/");
    cout << "running a depth-1 experiment on test 0" << endl;
    double ret = e.run(1, 0);
    cout << "result: " << ret << endl;
}

int main() {
    test_build_asts();
    test_MNIST();
    return 0;
}
