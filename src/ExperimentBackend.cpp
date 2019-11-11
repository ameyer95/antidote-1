#include "ExperimentBackend.h"
#include "AbstractSemanticsInstantiations.hpp"
#include "ASTNode.h"
#include "ConcreteSemantics.h"
#include "DropoutDomains.hpp"
#include "Feature.hpp"
#include <string>
#include <utility>
#include <vector>
using namespace std;

ExperimentBackend::ExperimentBackend(const DataSet *training, const DataSet *test) {
    this->training = training;
    this->test = test;
}

CategoricalDistribution<double> ExperimentBackend::run_concrete(int depth, int test_index) {
    ProgramNode *program = buildTree(depth);
    ConcreteSemantics sem;
    auto ret = sem.execute(test->rows[test_index].x, training, program);
    delete program;
    return ret;
}

CategoricalDistribution<Interval<double>> ExperimentBackend::run_abstract(int depth, int test_index, int num_dropout) {
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
    auto ret = sem.execute(test_input, initial_state, program);
    delete program;
    return ret.posterior_distribution_abstraction;
}

CategoricalDistribution<Interval<double>> ExperimentBackend::run_abstract_disjuncts(int depth, int test_index, int num_dropout) {
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
    auto ret = sem.execute(test_input, initial_state, program);
    delete program;
    std::vector<CategoricalDistribution<Interval<double>>> posteriors;
    for(auto i = ret.cbegin(); i != ret.cend(); i++) {
        posteriors.push_back(i->posterior_distribution_abstraction);
    }
    return d.D_domain.join(posteriors);
}

CategoricalDistribution<Interval<double>> ExperimentBackend::run_abstract_bounded_disjuncts(int depth, int test_index, int num_dropout, int disjunct_bound, const DisjunctsMergeMode &merge_mode) {
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
    auto ret = sem.execute(test_input, initial_state, program);
    delete program;
    std::vector<CategoricalDistribution<Interval<double>>> posteriors;
    for(auto i = ret.cbegin(); i != ret.cend(); i++) {
        posteriors.push_back(i->posterior_distribution_abstraction);
    }
    return d.D_domain.join(posteriors);
}
