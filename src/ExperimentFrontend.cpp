#include "ExperimentFrontend.h"
#include "CategoricalDistribution.hpp"
#include "ExperimentBackend.h"
#include "ExperimentDataWrangler.h"
#include "Interval.h"
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

/**
 * Auxiliary functions
 */

std::string setToString(const std::set<std::string> &options) {
    std::string ret = "{";
    for(auto i = options.cbegin(); i != options.cend(); i++) {
        if(i == options.cbegin()) {
            ret += *i;
        } else {
            ret += ", " + *i;
        }
    }
    ret += "}";
    return ret;
}

void vectorizeIntStringSplit(std::vector<int> &items, const std::string &space_separated_list) {
    std::istringstream iss(space_separated_list);
    for(std::string s; iss >> s; ) {
        items.push_back(std::stoi(s));
    }
}

std::string formatDistribution(const CategoricalDistribution<Interval<double>> &dist, const std::vector<std::string> &labels) {
    // XXX strong assumption that dist.size() == labels.size()
    std::string ret = "{";
    for(unsigned int i = 0; i < dist.size(); i++) {
        if(i != 0) {
            ret += ", ";
        }
        ret += labels[i] + ":" + to_string(dist[i]);
    }
    ret += "}";
    return ret;
}

std::string formatDistribution(const CategoricalDistribution<double> &dist, const std::vector<std::string> &labels) {
    std::string ret = "{";
    for(unsigned int i = 0; i < dist.size(); i++) {
        if(i != 0) {
            ret += ", ";
        }
        ret += labels[i] + ":" + std::to_string(dist[i]);
    }
    ret += "}";
    return ret;
}

/**
 * ExperimentFrontend member functions
 */

ExperimentFrontend::ExperimentFrontend() {
    createCommandLineArguments();
}

void ExperimentFrontend::createCommandLineArguments() {
    const std::set<std::string> dataset_options = strings_of_ExperimentDataEnum();
    const std::set<std::string> merge_options = strings_of_DisjunctsMergeMode();

    p.createArgument("depth", "-d", 1, "Space-separated list of depths of the tree to be built");
    p.createArgument("test_all", "-T", 0, "Run on each element in the test set", true);
    p.createArgument("test_indices", "-t", 1, "Space-separated list of test indices", true);
    p.createArgument("dataset", "-f", 2, "Dataset information: (1) the path to the data folder and (2) the name from one of " + setToString(dataset_options));
    p.createArgument("use_abstract", "-a", 1, "Use abstract semantics (not concrete); The passed value is a space-separated list of the n in <T,n>", true);
    p.createArgument("use_disjuncts", "-V", 1, "Like -a, but with disjuncts", true);
    p.createArgument("disjunct_bound", "-b", 2, "When -V is used, (1) an integer bound on the number of disjuncts, and (2) specify the merging strategy from " + setToString(merge_options), true);
    
    p.requireAtLeastOne({"test_all", "test_indices"});
    p.requireAtMostOne({"test_all", "test_indices"});
    p.requireAtMostOne({"use_abstract", "use_disjuncts"});

    p.requireTokenInSet("disjunct_bound", 1, merge_options);
    p.requireTokenInSet("dataset", 1, dataset_options);
}

void ExperimentFrontend::performSingleTest(int depth, int test_index) {
    if(test_index < e->test_size()) {
        if(params.use_abstract) {
            for(auto n = params.num_dropouts.cbegin(); n != params.num_dropouts.cend(); n++) {
                std::cout << "running a depth-" << depth << " experiment using <T," << *n << "> ";
                if(params.with_disjuncts) {
                    if(params.disjunct_bound.has_value()) {
                        std::cout << "(with disjuncts # <= " << params.disjunct_bound.value() << ") ";
                    } else {
                        std::cout << "(with disjuncts) ";
                    }
                }
                std::cout << "on test " << test_index << std::endl;
                ExperimentBackend::Result<Interval<double>> ret;
                if(!params.with_disjuncts) {
                    ret = e->run_abstract(depth, test_index, *n);
                } else {
                    if(params.disjunct_bound.has_value()) {
                        ret = e->run_abstract_bounded_disjuncts(depth, test_index, *n, params.disjunct_bound.value(), params.merge_mode);
                    } else {
                        ret = e->run_abstract_disjuncts(depth, test_index, *n);
                    }
                }
                std::cout << "result: " << formatDistribution(ret.posterior, current_data->class_labels)
                    << " (ground truth: " << current_data->class_labels[ret.ground_truth] << ")" << std::endl;
            }
        } else {
            std::cout << "running a depth-" << depth << " experiment using T on test " << test_index << std::endl;
            ExperimentBackend::Result<double> ret = e->run_concrete(depth, test_index);
            std::cout << "result: " << formatDistribution(ret.posterior, current_data->class_labels)
                << " (ground truth: " << current_data->class_labels[ret.ground_truth] << ")" << std::endl;
        }
    } else {
        std::cout << "skipping test " << test_index << " (out of bounds)" << std::endl;
    }
}

bool ExperimentFrontend::processCommandLineArguments(int argc, char ** const &argv) {
    p.parse(argc, argv);

    if(!p.failure()) {
        vectorizeIntStringSplit(params.depths, p["depth"].tokens[0]);
        params.test_all = p["test_all"].included;
        if(!params.test_all) {
            vectorizeIntStringSplit(params.test_indices, p["test_indices"].tokens[0]);
        }
        params.data_prefix = p["dataset"].tokens[0];
        params.dataset = string_to_ExperimentDataEnum(p["dataset"].tokens[1]);
        params.use_abstract = p["use_abstract"].included || p["use_disjuncts"].included;
        if(p["use_abstract"].included) {
            vectorizeIntStringSplit(params.num_dropouts, p["use_abstract"].tokens[0]);
            params.with_disjuncts = false;
        } else if(p["use_disjuncts"].included) {
            vectorizeIntStringSplit(params.num_dropouts, p["use_disjuncts"].tokens[0]);
            params.with_disjuncts = true;
            if(p["disjunct_bound"].included) {
                params.disjunct_bound = std::stoi(p["disjunct_bound"].tokens[0]);
                params.merge_mode = string_to_DisjunctsMergeMode(p["disjunct_bound"].tokens[1]);
            } else {
                params.disjunct_bound = {};
            }
        }
        return true;
    } else {
        std::cout << p.message() << std::endl;
        std::cout << p.help_string() << std::endl;
        return false;
    }
}

void ExperimentFrontend::performExperiments() {
    wrangler = new ExperimentDataWrangler(params.data_prefix);
    current_data = wrangler->fetch(params.dataset);
    e = new ExperimentBackend(current_data->training, current_data->test);
    for(auto depth = params.depths.begin(); depth != params.depths.end(); depth++) {
        if(params.test_all) {
            for(int i = 0; i < e->test_size(); i++) {
                performSingleTest(*depth, i);
            }
        } else {
            for(auto i = params.test_indices.begin(); i != params.test_indices.end(); i++) {
                performSingleTest(*depth, *i);
            }
        }
    }
    delete e;
    delete wrangler;
}
