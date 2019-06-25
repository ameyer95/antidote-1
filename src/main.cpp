#include "ArgParse.h"
#include "ASTNode.h"
#include "MNISTExperiment.h"
#include "MNIST.h"
#include "PrettyPrinter.h"
#include <cstdint>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
using namespace std;

struct RunParams {
    int depth;
    vector<int> test_indices;
    string mnist_prefix;
};

/**
 * Forward declarations for main
 */
bool readParams(RunParams &params, const int &argc, char ** const &argv);
void test_MNIST(const RunParams &params);

int main(int argc, char **argv) {
    RunParams params;
    if(readParams(params, argc, argv)) {
        test_MNIST(params);
    }
    return 0;
}


/**
 * Relevant code
 */

void vectorizeIntStringSplit(vector<int> &items, const string &space_separated_list) {
    istringstream iss(space_separated_list);
    for(string s; iss >> s; ) {
        items.push_back(stoi(s));
    }
}

bool readParams(RunParams &params, const int &argc, char ** const &argv) {
    Argument *depth, *test_indices, *mnist_prefix;
    ArgParse p;
    bool success;

    depth = p.createArgument("-d", 1, "Depth of the tree to be built");
    test_indices = p.createArgument("-t", 1, "Space-separated list of test indices");
    mnist_prefix = p.createArgument("-m", 1, "Path to MNIST datasets");
    p.parse(argc, argv);

    if(!p.failure()) {
        params.depth = stoi(depth->tokens[0]);
        vectorizeIntStringSplit(params.test_indices, test_indices->tokens[0]);
        params.mnist_prefix = mnist_prefix->tokens[0];
        success = true;
    } else {
        cout << p.message() << endl;
        cout << p.help_string() << endl;
        success = false;
    }

    delete depth;
    delete test_indices;
    delete mnist_prefix;
    return success;
}

void test_MNIST(const RunParams &params) {
    MNISTExperiment e(params.mnist_prefix);
    for(vector<int>::const_iterator i = params.test_indices.begin(); i != params.test_indices.end(); i++) {
        if(*i < e.test_size()) {
            cout << "running a depth-" << params.depth << " experiment on test " << *i << endl;
            double ret = e.run(params.depth, *i);
            cout << "result: " << ret << endl;
        } else {
            cout << "skipping test " << *i << " (out of bounds)" << endl;
        }
    }
}


/**
 * (Now-)unused prototype code
 */

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
