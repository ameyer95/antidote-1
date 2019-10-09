#include "ArgParse.h"
#include "ASTNode.h"
#include "MNISTExperiment.h"
#include "MNIST.h"
#include "PrettyPrinter.h"
#include <cstdint>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
using namespace std;

// In tandem with readParams, defines the command-line flags
struct RunParams {
    vector<int> depths;
    bool test_all; // When false, use only the indices in test_indices
    vector<int> test_indices;
    string mnist_prefix;
    bool use_abstract; // When false, use concrete semantics
    bool with_disjuncts; // When true, use_abstract must also be true, and this says to do the more precise domain
    vector<int> num_dropouts; // For when use_abstract == true
    optional<int> disjunct_bound; // Optionally, has_value only when with_disjuncts is true
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

// In tandem with RunParams, defines the command-line flags
bool readParams(RunParams &params, const int &argc, char ** const &argv) {
    Argument *depth, *test_all, *test_indices, *mnist_prefix, *use_abstract, *use_disjuncts, *disjunct_bound;
    ArgParse p;
    bool success;

    depth = p.createArgument("-d", 1, "Space-separated list of depths of the tree to be built");
    test_all = p.createArgument("-T", 0, "Run on each element in the test set", true);
    test_indices = p.createArgument("-t", 1, "Space-separated list of test indices", true);
    mnist_prefix = p.createArgument("-m", 1, "Path to MNIST datasets");
    use_abstract = p.createArgument("-a", 1, "Use abstract semantics (not concrete); The passed value is a space-separated list of the n in <T,n>", true);
    use_disjuncts = p.createArgument("-V", 1, "Like -a, but with disjuncts", true);
    disjunct_bound = p.createArgument("-b", 1, "When -V is used, an integer bound on the number of disjuncts", true);
    
    p.parse(argc, argv);

    if(!p.failure()) {
        if(!test_all->included && !test_indices->included) {
            cout << "Must specify test cases with -t or -T" << endl;
            success = false;
        } else if(use_abstract->included && use_disjuncts->included) {
            cout << "Must specify only one of -a and -V" << endl;
            success = false;
        } else {
            vectorizeIntStringSplit(params.depths, depth->tokens[0]);
            params.test_all = test_all->included;
            if(!params.test_all) {
                vectorizeIntStringSplit(params.test_indices, test_indices->tokens[0]);
            }
            params.mnist_prefix = mnist_prefix->tokens[0];
            params.use_abstract = use_abstract->included || use_disjuncts->included;
            if(use_abstract->included) {
                vectorizeIntStringSplit(params.num_dropouts, use_abstract->tokens[0]);
                params.with_disjuncts = false;
            } else if(use_disjuncts->included) {
                vectorizeIntStringSplit(params.num_dropouts, use_disjuncts->tokens[0]);
                params.with_disjuncts = true;
                if(disjunct_bound->included) {
                    params.disjunct_bound = stoi(disjunct_bound->tokens[0]);
                } else {
                    params.disjunct_bound = {};
                }
            }
            success = true;
        }
    } else {
        cout << p.message() << endl;
        cout << p.help_string() << endl;
        success = false;
    }

    // TODO make ArgParse maintain some kind of map to avoid this
    delete depth;
    delete test_all;
    delete test_indices;
    delete mnist_prefix;
    delete use_abstract;
    delete use_disjuncts;
    delete disjunct_bound;
    return success;
}

inline void perform_single_test(const RunParams &params, MNISTExperiment &e, int depth, int index) {
    if(index < e.test_size()) {
        if(params.use_abstract) {
            for(vector<int>::const_iterator n = params.num_dropouts.begin(); n != params.num_dropouts.end(); n++) {
                cout << "running a depth-" << depth << " experiment using <T," << *n << "> ";
                if(params.with_disjuncts) {
                    if(params.disjunct_bound.has_value()) {
                        cout << "(with disjuncts # <= " << params.disjunct_bound.value() << ") ";
                    } else {
                        cout << "(with disjuncts) ";
                    }
                }
                cout << "on test " << index << endl;
                Interval<double> ret;
                if(!params.with_disjuncts) {
                    ret = e.run_abstract(depth, index, *n);
                } else {
                    if(params.disjunct_bound.has_value()) {
                        ret = e.run_abstract_bounded_disjuncts(depth, index, *n, params.disjunct_bound.value());
                    } else {
                        ret = e.run_abstract_disjuncts(depth, index, *n);
                    }
                }
                cout << "result: " << to_string(ret) << endl;
            }
        } else {
            cout << "running a depth-" << depth << " experiment using T on test " << index << endl;
            double ret = e.run_concrete(depth, index);
            cout << "result: " << ret << endl;
        }
    } else {
        cout << "skipping test " << index << " (out of bounds)" << endl;
    }
}

void test_MNIST(const RunParams &params) {
    MNISTExperiment e(params.mnist_prefix);
    for(vector<int>::const_iterator depth = params.depths.begin(); depth != params.depths.end(); depth++) {
        if(params.test_all) {
            for(int i = 0; i < e.test_size(); i++) {
                perform_single_test(params, e, *depth, i);
            }
        } else {
            for(vector<int>::const_iterator i = params.test_indices.begin(); i != params.test_indices.end(); i++) {
                perform_single_test(params, e, *depth, *i);
            }
        }
    }
}


/**
 * (Now-)unused prototype code
 */

void test_build_ast(int depth) {
    ProgramNode *root = buildTree(depth);
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
