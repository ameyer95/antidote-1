#include "ASTNode.h"
#include "PrettyPrinter.h"
#include "MNISTReader.h"
#include <iostream>
#include <utility>
using namespace std;


void test_build_ast(int depth) {
    ASTNode *root = ASTNode::buildTree(depth);
    PrettyPrinter p = PrettyPrinter();
    cout << "Depth " << depth << endl;
    root->accept(p);
    cout << endl;
}

void test_build_asts() {
    for(int i = 0; i < 3; i++) {
        test_build_ast(i);
    }
}

void test_load_MNIST() {
    pair<LabelFile, ImageFile> training, test;
    training = MNIST_readTrainingSet("data/");
    test = MNIST_readTestSet("data/");
}

int main() {
    test_build_asts();
    test_load_MNIST();
    return 0;
}
