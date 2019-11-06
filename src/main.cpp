#include "ArgParse.h"
#include "ASTNode.h"
#include "MNISTExperiment.h"
#include "MNIST.h"
#include "PrettyPrinter.h"
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
    string merge_mode; // For when disjunct_bound.has_value(), takes value "greedy" or "optimal"
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
    ArgParse p;
    bool success;

    p.createArgument("depth", "-d", 1, "Space-separated list of depths of the tree to be built");
    p.createArgument("test_all", "-T", 0, "Run on each element in the test set", true);
    p.createArgument("test_indices", "-t", 1, "Space-separated list of test indices", true);
    p.createArgument("mnist_prefix", "-m", 1, "Path to MNIST datasets");
    p.createArgument("use_abstract", "-a", 1, "Use abstract semantics (not concrete); The passed value is a space-separated list of the n in <T,n>", true);
    p.createArgument("use_disjuncts", "-V", 1, "Like -a, but with disjuncts", true);
    p.createArgument("disjunct_bound", "-b", 2, "When -V is used, (1) an integer bound on the number of disjuncts, and (2) \"greedy\" or \"optimal\" to specify the merging strategy", true);
    
    p.requireAtLeastOne({"test_all", "test_indices"});
    p.requireAtMostOne({"test_all", "test_indices"});
    p.requireAtMostOne({"use_abstract", "use_disjuncts"});

    p.requireTokenConstraint("disjunct_bound", 1, [](const std::string &value){ return value == "greedy" || value == "optimal"; }, "Second argument of -b must be either \"greedy\" or \"optimal\"");

    p.parse(argc, argv);

    if(!p.failure()) {
        success = true;
        vectorizeIntStringSplit(params.depths, p["depth"].tokens[0]);
        params.test_all = p["test_all"].included;
        if(!params.test_all) {
            vectorizeIntStringSplit(params.test_indices, p["test_indices"].tokens[0]);
        }
        params.mnist_prefix = p["mnist_prefix"].tokens[0];
        params.use_abstract = p["use_abstract"].included || p["use_disjuncts"].included;
        if(p["use_abstract"].included) {
            vectorizeIntStringSplit(params.num_dropouts, p["use_abstract"].tokens[0]);
            params.with_disjuncts = false;
        } else if(p["use_disjuncts"].included) {
            vectorizeIntStringSplit(params.num_dropouts, p["use_disjuncts"].tokens[0]);
            params.with_disjuncts = true;
            if(p["disjunct_bound"].included) {
                params.disjunct_bound = stoi(p["disjunct_bound"].tokens[0]);
                params.merge_mode = p["disjunct_bound"].tokens[1];
            } else {
                params.disjunct_bound = {};
            }
        }
    } else {
        cout << p.message() << endl;
        cout << p.help_string() << endl;
        success = false;
    }

    return success;
}

inline void perform_single_test(const RunParams &params, MNISTExperiment &e, int depth, int index) {
    if(index < e.test_size()) {
        if(params.use_abstract) {
            for(auto n = params.num_dropouts.cbegin(); n != params.num_dropouts.cend(); n++) {
                cout << "running a depth-" << depth << " experiment using <T," << *n << "> ";
                if(params.with_disjuncts) {
                    if(params.disjunct_bound.has_value()) {
                        cout << "(with disjuncts # <= " << params.disjunct_bound.value() << ") ";
                    } else {
                        cout << "(with disjuncts) ";
                    }
                }
                cout << "on test " << index << endl;
                CategoricalDistribution<Interval<double>> ret;
                if(!params.with_disjuncts) {
                    ret = e.run_abstract(depth, index, *n);
                } else {
                    if(params.disjunct_bound.has_value()) {
                        ret = e.run_abstract_bounded_disjuncts(depth, index, *n, params.disjunct_bound.value(), params.merge_mode);
                    } else {
                        ret = e.run_abstract_disjuncts(depth, index, *n);
                    }
                }
                cout << "result: {0:" << to_string(ret[0]) << ", 1:" << to_string(ret[1])
                    << "} (ground truth: " << e.groundTruth(index) << ")" << endl;
            }
        } else {
            cout << "running a depth-" << depth << " experiment using T on test " << index << endl;
            CategoricalDistribution<double> ret = e.run_concrete(depth, index);
            cout << "result: {0:" << ret[0] << ", 1:" << ret[1] << "} (ground truth: " << e.groundTruth(index) << ")" << endl;
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
