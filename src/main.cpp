#include "ASTNode.h"
#include "PrettyPrinter.h"
#include <iostream>
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

int main() {
    test_build_asts();
    return 0;
}
