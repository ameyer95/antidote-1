#include "ExperimentBackend.h"
#include "AbstractSemanticsInstantiations.hpp"
#include "ASTNode.h"
#include "ConcreteSemantics.h"
#include "DropoutDomains.hpp"
#include "DropoutDomainsLabels.hpp"
#include "Feature.hpp"
#include <algorithm>
#include <cstdlib> // for random stuff
#include <map>
#include <string>
#include <utility>
#include <vector>
using namespace std;

/**
 * Auxiliary functions
 */

// Returns a set since there may be a tie, even in the concrete case.
// Could be determinized by taking the min class label.
set<int> softMax(const CategoricalDistribution<double> &p) {
    set<int> ret;
    double max_value = *max_element(p.cbegin(), p.cend());
    for(int i = 0; i < p.size(); i++) {
        if(p[i] >= max_value) {
            ret.insert(i);
        }
    }
    return ret;
}

// Any element whose max value is at least the largest-over-elements'-min-values
// could feasibly be the maximum
set<int> softMax(const CategoricalDistribution<Interval<double>> &p) {
    set<int> ret;
    double threshold = max_element(p.cbegin(), p.cend(),
            [](const Interval<double> &e1, const Interval<double> &e2)
                { return e1.get_lower_bound() < e2.get_lower_bound(); }
            )->get_lower_bound();
    for(int i = 0; i < p.size(); i++) {
        if(p[i].get_upper_bound() >= threshold) {
            ret.insert(i);
        }
    }
    return ret;
}

unsigned int random_removal_size(int set_size, int num_dropout) {
    // TODO make this actually consider smaller amounts
    return num_dropout;
}

DataReferences* random_subset(const DataSet *training, int num_dropout) {
    unsigned int removal_size = random_removal_size(training->rows.size(), num_dropout);
    set<int> indices;
    while(indices.size() < removal_size) {
        indices.insert(rand() % training->rows.size());
    }
    DataReferences *ret = new DataReferences(training);
    // For each index in decreasing order, remove it
    for(auto i = indices.crbegin(); i != indices.crend(); i++) {
        ret->remove(*i);
    }
    return ret;
}

/**
 * ExperimentBackend members
 */

ExperimentBackend::ExperimentBackend(const DataSet *training, const DataSet *test, bool label_flipping) {
    this->training = training;
    this->test = test;
    this->use_label_flipping = label_flipping;
}

ExperimentBackend::Result<double> ExperimentBackend::run_concrete(int depth, int test_index) {
    ProgramNode *program = buildTree(depth);
    ConcreteSemantics sem;
    auto ret = sem.execute(test->rows[test_index].x, training, program);
    delete program;
    return { ret, softMax(ret), groundTruth(test_index) };
}

ExperimentBackend::Result<Interval<double>> ExperimentBackend::run_abstract(int depth, int test_index, int num_dropout) {
    ProgramNode *program = buildTree(depth);
    if (this->use_label_flipping) {
        DropoutDomainsLabels d;
        BoxDropoutSemanticsLabels sem(&d.box_domain);
        FeatureVector test_input = test->rows[test_index].x;
        DataReferences training_references(training);
        BoxDropoutDomainLabels::AbstractionType initial_state = {
            TrainingReferencesWithDropoutLabels(training_references, num_dropout),
            PredicateAbstractionLabels(1), // XXX any non-bot value, ideally top?
            PosteriorDistributionAbstractionLabels(1) // XXX any non-bot value, ideally top?
        };
        auto final_state = sem.execute(test_input, initial_state, program);
        auto ret = final_state.posterior_distribution_abstraction;
        delete program;
        return { ret, softMax(ret), groundTruth(test_index) };
    } else {
        DropoutDomains d;
        BoxDropoutSemantics sem(&d.box_domain);
        FeatureVector test_input = test->rows[test_index].x;
        DataReferences training_references(training);
        BoxDropoutDomain::AbstractionType initial_state = {
            TrainingReferencesWithDropout(training_references, num_dropout),
            PredicateAbstraction(1), // XXX any non-bot value, ideally top?
            PosteriorDistributionAbstraction(1) // XXX any non-bot value, ideally top?
        };
        auto final_state = sem.execute(test_input, initial_state, program);
        auto ret = final_state.posterior_distribution_abstraction;
        delete program;
        return { ret, softMax(ret), groundTruth(test_index) };
    }


}

ExperimentBackend::Result<Interval<double>> ExperimentBackend::run_abstract_disjuncts(int depth, int test_index, int num_dropout) {
    ProgramNode *program = buildTree(depth);
    DropoutDomains d;
    BoxDisjunctsDropoutSemantics sem(&d.disjuncts_domain);
    FeatureVector test_input = test->rows[test_index].x;
    DataReferences training_references(training);
    BoxDropoutDomain::AbstractionType initial_box = {
        TrainingReferencesWithDropout(training_references, num_dropout),
        PredicateAbstraction(1), // XXX any non-bot value, ideally top?
        PosteriorDistributionAbstraction(1) // XXX any non-bot value, ideally top?
    };
    BoxDisjunctsDomainDropoutInstantiation::AbstractionType initial_state = {initial_box};
    auto final_state = sem.execute(test_input, initial_state, program);
    delete program;
    std::vector<CategoricalDistribution<Interval<double>>> posteriors;
    for(auto i = final_state.cbegin(); i != final_state.cend(); i++) {
        posteriors.push_back(i->posterior_distribution_abstraction);
    }
    auto ret = d.D_domain.join(posteriors);
    return { ret, softMax(ret), groundTruth(test_index) };
}

ExperimentBackend::Result<Interval<double>> ExperimentBackend::run_abstract_bounded_disjuncts(int depth, int test_index, int num_dropout, int disjunct_bound, const DisjunctsMergeMode &merge_mode) {
    ProgramNode *program = buildTree(depth);
    DropoutDomains d;
    FeatureVector test_input = test->rows[test_index].x;

    d.bounded_disjuncts_domain.setMergeDetails(disjunct_bound, merge_mode);
    BoxDisjunctsDropoutSemantics sem(&d.bounded_disjuncts_domain);
    DataReferences training_references(training);
    BoxDropoutDomain::AbstractionType initial_box = {
        TrainingReferencesWithDropout(training_references, num_dropout),
        PredicateAbstraction(1), // XXX any non-bot value, ideally top?
        PosteriorDistributionAbstraction(1) // XXX any non-bot value, ideally top?
    };
    BoxDisjunctsDomainDropoutInstantiation::AbstractionType initial_state = {initial_box};
    auto final_state = sem.execute(test_input, initial_state, program);
    delete program;
    std::vector<CategoricalDistribution<Interval<double>>> posteriors;
    for(auto i = final_state.cbegin(); i != final_state.cend(); i++) {
        posteriors.push_back(i->posterior_distribution_abstraction);
    }
    auto ret = d.D_domain.join(posteriors);
    return { ret, softMax(ret), groundTruth(test_index) };
}

std::map<int,int> ExperimentBackend::run_test(int depth, int test_index, int num_dropout, int num_trials, unsigned int seed) {
    ProgramNode *program = buildTree(depth);
    ConcreteSemantics sem;
    map<int,int> ret;
    for(int i = 0; i < training->num_categories; i++) {
        ret.insert(make_pair(i, 0));
    }
    srand(seed);
    for(int i = 0; i < num_trials; i++) {
        DataReferences *subset = random_subset(training, num_dropout);
        auto result = sem.execute(test->rows[test_index].x, subset, program);
        set<int> classification = softMax(result);
        for(auto j = classification.cbegin(); j != classification.cend(); j++) {
            ret[*j]++;
        }
        delete subset;
    }
    delete program;
    return ret;
}
