#include "ASTNode.h"
#include "PrettyPrinter.h"
#include "Experiment.h"
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

void test_MNIST() {
    Experiment e("data/");
    cout << "running a depth-1 experiment on test 0" << endl;
    double ret = e.run(1, 0);
    cout << "result: " << ret << endl;
}

int main() {
    test_build_asts();
    test_MNIST();
    return 0;
}
