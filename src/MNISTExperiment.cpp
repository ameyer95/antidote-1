#include "MNISTExperiment.h"
#include "AbstractSemanticsInstantiations.hpp"
#include "ASTNode.h"
#include "ConcreteSemantics.h"
#include "DropoutDomains.hpp"
#include "Feature.hpp"
#include <string>
#include <utility>
#include <vector>
using namespace std;

MNISTExperiment::MNISTExperiment(ExperimentDataWrangler *wrangler) {
    this->wrangler = wrangler;
    const ExperimentData *mnist = wrangler->fetch(ExperimentDataEnum::MNIST_BOOLEAN_1_7);
    this->mnist_training = mnist->training;
    this->mnist_test = mnist->test;
}

CategoricalDistribution<double> MNISTExperiment::run_concrete(int depth, int test_index) {
    ProgramNode *program = buildTree(depth);
    ConcreteSemantics sem;
    auto ret = sem.execute(mnist_test->rows[test_index].x, mnist_training, program);
    delete program;
    return ret;
}

CategoricalDistribution<Interval<double>> MNISTExperiment::run_abstract(int depth, int test_index, int num_dropout) {
    ProgramNode *program = buildTree(depth);
    DropoutDomains d;
    BoxDropoutSemantics sem(&d.box_domain);
    FeatureVector test_input = mnist_test->rows[test_index].x;
    DataReferences training_references(mnist_training);
    BoxDropoutDomain::AbstractionType initial_state = {
        TrainingReferencesWithDropout(training_references, num_dropout),
        PredicateAbstraction(1), // XXX any non-bot value, ideally top?
        PosteriorDistributionAbstraction(1) // XXX any non-bot value, ideally top?
    };
    auto ret = sem.execute(test_input, initial_state, program);
    delete program;
    return ret.posterior_distribution_abstraction;
}

CategoricalDistribution<Interval<double>> MNISTExperiment::run_abstract_disjuncts(int depth, int test_index, int num_dropout) {
    ProgramNode *program = buildTree(depth);
    DropoutDomains d;
    BoxDisjunctsDropoutSemantics sem(&d.disjuncts_domain);
    FeatureVector test_input = mnist_test->rows[test_index].x;
    DataReferences training_references(mnist_training);
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

CategoricalDistribution<Interval<double>> MNISTExperiment::run_abstract_bounded_disjuncts(int depth, int test_index, int num_dropout, int disjunct_bound, const std::string &merge_mode) {
    ProgramNode *program = buildTree(depth);
    DropoutDomains d;
    FeatureVector test_input = mnist_test->rows[test_index].x;
    typedef BoxBoundedDisjunctsDomainDropoutInstantiation::MergeMode MMode;
    MMode merge_mode_enum = (merge_mode == "optimal" ? MMode::OPTIMAL : MMode::GREEDY);

    d.bounded_disjuncts_domain.setMergeDetails(disjunct_bound, merge_mode_enum);
    BoxDisjunctsDropoutSemantics sem(&d.bounded_disjuncts_domain);
    DataReferences training_references(mnist_training);
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
