#include "ExperimentFrontend.h"
#include "CategoricalDistribution.hpp"
#include "CommonEnums.h"
#include "ExperimentBackend.h"
#include "ExperimentDataWrangler.h"
#include "Interval.h"
#include <iostream>
#include <map>
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
    p.createArgument("verbose", "-v", 0, "", true);
    p.createArgument("random_test", "-r", 3, "Run concrete semantics on random samples from <T,n>. (1) the value of n, (2) the number of random samples, (3) the random seed.", true);
    
    p.requireAtLeastOne({"test_all", "test_indices"});
    p.requireAtMostOne({"test_all", "test_indices"});
    p.requireAtMostOne({"use_abstract", "use_disjuncts", "random_test"});

    p.requireTokenInSet("disjunct_bound", 1, merge_options);
    p.requireTokenInSet("dataset", 1, dataset_options);
}

void ExperimentFrontend::performSingleTest(int depth, int test_index) {
    if(test_index < e->test_size()) {
        if(params.random_test.flag) {
            output("running a depth-" + std::to_string(depth) + " random test (" + std::to_string(params.random_test.num_trials) + ") using <T," + std::to_string(params.random_test.num_dropout) + "> on test " + std::to_string(test_index));
            std::map<int,int> ret = e->run_test(depth, test_index, params.random_test.num_dropout, params.random_test.num_trials, params.random_test.seed);
            output(output_to_json(depth, test_index, ret), true);
        } else if(params.use_abstract) {
            performAbstractTests(depth, test_index);
        } else {
            output("running a depth-" + std::to_string(depth) + " experiment using T on test " + std::to_string(test_index));
            ExperimentBackend::Result<double> ret = e->run_concrete(depth, test_index);
            output(output_to_json(depth, test_index, ret), true);
        }
    } else {
        output("skipping test " + std::to_string(test_index) + " (out of bounds)");
    }
}

void ExperimentFrontend::performAbstractTests(int depth, int test_index) {
    for(auto n = params.num_dropouts.cbegin(); n != params.num_dropouts.cend(); n++) {
        std::string message = "running a depth-" + std::to_string(depth) + " experiment using <T," + std::to_string(*n) + "> ";
        if(params.with_disjuncts) {
            if(params.disjunct_bound.has_value()) {
                message += "(with disjuncts # <= " + std::to_string(params.disjunct_bound.value()) + ") ";
            } else {
                message += "(with disjuncts) ";
            }
        }
        message += "on test " + std::to_string(test_index);
        output(message);
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
        output(output_to_json(depth, test_index, ret), true);
    }
}

std::string ExperimentFrontend::output_to_json(int depth, int test_index, const std::map<int,int> &result) {
    std::string ret = "{ ";
    ret += "\"depth\" : " + std::to_string(depth) + ", ";
    ret += "\"test_index\" : " + std::to_string(test_index) + ", ";
    ret += "\"classification_counts\" : { ";
    for(auto i = result.cbegin(); i != result.cend(); i++) {
        if(i->second != 0) {
            if(i != result.cbegin()) {
                ret += ", ";
            }
            ret += "\"" + current_data->class_labels[i->first] + "\" : " + std::to_string(i->second);
        }
    }
    ret += " }";
    ret += " }";
    return ret;
}

std::string ExperimentFrontend::output_to_json(int depth, int test_index, const ExperimentBackend::Result<double> &result) {
    std::string ret = "{ ";
    ret += "\"depth\" : " + std::to_string(depth) + ", ";
    ret += "\"test_index\" : " + std::to_string(test_index) + ", ";
    ret += "\"ground_truth\" : \"" + current_data->class_labels[result.ground_truth] + "\", ";
    ret += "\"posterior\" : { ";
    for(unsigned int i = 0; i < result.posterior.size(); i++) {
        if(i != 0) {
            ret += ", ";
        }
        ret += "\"" + current_data->class_labels[i] + "\" : " + std::to_string(result.posterior[i]);
    }
    ret += " }, ";
    ret += "\"possible_classifications\" : [ ";
    for(auto i = result.possible_classifications.cbegin(); i != result.possible_classifications.cend(); i++) {
        if(i != result.possible_classifications.cbegin()) {
            ret += ", ";
        }
        ret += "\"" + current_data->class_labels[*i] + "\"";
    }
    ret += " ]";
    ret += " }";
    return ret;
}

std::string ExperimentFrontend::output_to_json(int depth, int test_index, const ExperimentBackend::Result<Interval<double>> &result) {
    std::string ret = "{ ";
    ret += "\"depth\" : " + std::to_string(depth) + ", ";
    ret += "\"test_index\" : " + std::to_string(test_index) + ", ";
    ret += "\"ground_truth\" : \"" + current_data->class_labels[result.ground_truth] + "\", ";
    ret += "\"posterior\" : { ";
    for(unsigned int i = 0; i < result.posterior.size(); i++) {
        if(i != 0) {
            ret += ", ";
        }
        ret += "\"" + current_data->class_labels[i] + "\" : ";
        ret += "[ " + std::to_string(result.posterior[i].get_lower_bound()) + ", "
            + std::to_string(result.posterior[i].get_upper_bound()) + " ]";
    }
    ret += " }, ";
    ret += "\"possible_classifications\" : [ ";
    for(auto i = result.possible_classifications.cbegin(); i != result.possible_classifications.cend(); i++) {
        if(i != result.possible_classifications.cbegin()) {
            ret += ", ";
        }
        ret += "\"" + current_data->class_labels[*i] + "\"";
    }
    ret += " ]";
    ret += " }";
    return ret;
}

void ExperimentFrontend::output(const std::string &message, bool force) {
    if(verbose || force) {
        std::cout << message << std::endl;
    }
}

bool ExperimentFrontend::processCommandLineArguments(int argc, char ** const &argv) {
    p.parse(argc, argv);

    if(!p.failure()) {
        verbose = p["verbose"].included;
        vectorizeIntStringSplit(params.depths, p["depth"].tokens[0]);
        params.test_all = p["test_all"].included;
        if(!params.test_all) {
            vectorizeIntStringSplit(params.test_indices, p["test_indices"].tokens[0]);
        }
        params.data_prefix = p["dataset"].tokens[0];
        params.dataset = string_to_ExperimentDataEnum(p["dataset"].tokens[1]);
        params.random_test.flag = p["random_test"].included;
        params.use_abstract = p["use_abstract"].included || p["use_disjuncts"].included;
        if(p["random_test"].included) {
            params.random_test.num_dropout = std::stoi(p["random_test"].tokens[0]);
            params.random_test.num_trials = std::stoi(p["random_test"].tokens[1]);
            params.random_test.seed = std::stoi(p["random_test"].tokens[2]);
        } else if(p["use_abstract"].included) {
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
