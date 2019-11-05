#include "MNISTExperiment.h"
#include "AbstractSemanticsInstantiations.hpp"
#include "ASTNode.h"
#include "BoxBoundedDisjunctsDomainDropoutInstantiation.h"
#include "BoxDisjunctsDomainDropoutInstantiation.h"
#include "BoxStateDomainDropoutInstantiation.h"
#include "ConcreteSemantics.h"
#include "Feature.hpp"
#include "MNIST.h"
#include <string>
#include <utility>
#include <vector>
using namespace std;

// Uses 128 as the binary threshold (each pixel is a byte)
inline FeatureVector Image_to_Input(const Image &image) {
    FeatureVector ret(MNIST_IMAGE_SIZE);
    for(int i = 0; i < MNIST_IMAGE_SIZE; i++) {
        ret[i] = *(image + i) > 128;
    }
    return ret;
}

// The classes pair assigns the labels first -> 0 and second -> 1
DataSet* simplified(const RawMNIST &mnist, pair<int, int> classes = make_pair(1, 7)) {
    DataSet *ret = new DataSet { FeatureVectorHeader(MNIST_IMAGE_SIZE, FeatureType::BOOLEAN),
                                 2,
                                 vector<DataRow>(0) };

    for(unsigned int i = 0; i < mnist.size(); i++) {
        if(mnist[i].second == classes.first || mnist[i].second == classes.second) {
            DataRow temp = { Image_to_Input(mnist[i].first),
                             (mnist[i].second == classes.second ? 1 : 0) };
            ret->rows.push_back(temp);
        }
    }

    return ret;
}

MNISTExperiment::MNISTExperiment(string mnistPrefix) {
    // The following allocate and populate this->{mnist_training, mnist_test, predicates}
    // (Accordingly, that's what the destructor cleans up)
    loadMNIST(mnistPrefix);
}

MNISTExperiment::~MNISTExperiment() {
    delete mnist_training;
    delete mnist_test;
}

void MNISTExperiment::loadMNIST(string mnistPrefix) {
    RawMNIST raw_mnist_training(MNISTMode::TRAINING, mnistPrefix);
    RawMNIST raw_mnist_test(MNISTMode::TEST, mnistPrefix);
    mnist_training = simplified(raw_mnist_training);
    mnist_test = simplified(raw_mnist_test);
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
    FeatureVector test_input = mnist_test->rows[test_index].x;
    TrainingSetDropoutDomain L_T;
    PredicateSetDomain L_Phi;
    PosteriorDistributionIntervalDomain L_D;
    BoxDropoutDomain state_domain(&L_T, &L_Phi, &L_D);
    BoxDropoutSemantics sem(&state_domain);
    DataReferences training_references(mnist_training);
    BoxDropoutDomain::BoxStateAbstractionType initial_state = {
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
    FeatureVector test_input = mnist_test->rows[test_index].x;
    TrainingSetDropoutDomain L_T;
    PredicateSetDomain L_Phi;
    PosteriorDistributionIntervalDomain L_D;
    BoxDropoutDomain box_domain(&L_T, &L_Phi, &L_D);
    BoxDisjunctsDomainDropoutInstantiation state_domain(&box_domain);
    BoxDisjunctsDropoutSemantics sem(&state_domain);
    DataReferences training_references(mnist_training);
    BoxDropoutDomain::BoxStateAbstractionType initial_box = {
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
    return box_domain.posterior_distribution_domain->join(posteriors);
}

CategoricalDistribution<Interval<double>> MNISTExperiment::run_abstract_bounded_disjuncts(int depth, int test_index, int num_dropout, int disjunct_bound, const std::string &merge_mode) {
    ProgramNode *program = buildTree(depth);
    FeatureVector test_input = mnist_test->rows[test_index].x;
    TrainingSetDropoutDomain L_T;
    PredicateSetDomain L_Phi;
    PosteriorDistributionIntervalDomain L_D;
    BoxDropoutDomain box_domain(&L_T, &L_Phi, &L_D);
    BoxDisjunctsDomainDropoutInstantiation V_domain(&box_domain);
    typedef BoxBoundedDisjunctsDomainDropoutInstantiation::MergeMode MMode;
    MMode merge_mode_enum = (merge_mode == "optimal" ? MMode::OPTIMAL : MMode::GREEDY);
    BoxBoundedDisjunctsDomainDropoutInstantiation state_domain(&V_domain, disjunct_bound, merge_mode_enum);
    BoxDisjunctsDropoutSemantics sem(&state_domain);
    DataReferences training_references(mnist_training);
    BoxDropoutDomain::BoxStateAbstractionType initial_box = {
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
    return box_domain.posterior_distribution_domain->join(posteriors);
}
