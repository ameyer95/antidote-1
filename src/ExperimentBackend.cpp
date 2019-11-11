#include "ExperimentBackend.h"
#include "AbstractSemanticsInstantiations.hpp"
#include "ASTNode.h"
#include "ConcreteSemantics.h"
#include "DropoutDomains.hpp"
#include "Feature.hpp"
#include <algorithm>
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

/**
 * ExperimentBackend members
 */

ExperimentBackend::ExperimentBackend(const DataSet *training, const DataSet *test) {
    this->training = training;
    this->test = test;
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
