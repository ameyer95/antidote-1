#include "ConcreteSemantics.h"
#include "ASTNode.h"
#include "CategoricalDistribution.h"
#include "ConcreteTrainingReferences.h"
#include "Predicate.hpp"
using namespace std;

CategoricalDistribution ConcreteSemantics::execute(const FeatureVector &test_input, const DataSet *training_set, const ProgramNode *program) {
    this->test_input = test_input;
    current_state.training_references = ConcreteTrainingReferences(training_set);
    current_state.phi = {};
    program->accept(*this);
    return return_value;
}

void ConcreteSemantics::visit(const ProgramNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

void ConcreteSemantics::visit(const SequenceNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

void ConcreteSemantics::visit(const ITEImpurityNode &node) {
    if(current_state.training_references.isPure()) {
        node.get_left_child()->accept(*this);
    } else {
        node.get_right_child()->accept(*this);
    }
}

void ConcreteSemantics::visit(const ITENoPhiNode &node) {
    if(!current_state.phi.has_value()) {
        node.get_left_child()->accept(*this);
    } else {
        node.get_right_child()->accept(*this);
    }
}

void ConcreteSemantics::visit(const BestSplitNode &node) {
    current_state.phi = current_state.training_references.bestSplit();
}

void ConcreteSemantics::visit(const SummaryNode &node) {
    current_state.posterior = current_state.training_references.summary();
}

void ConcreteSemantics::visit(const UsePhiSequenceNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

void ConcreteSemantics::visit(const ITEModelsNode &node) {
    // Grammar guarantees current_state.phi.has_value()
    if(current_state.phi.value().evaluate(test_input)) {
        node.get_left_child()->accept(*this);
    } else {
        node.get_right_child()->accept(*this);
    }
}

void ConcreteSemantics::visit(const FilterNode &node) {
    // Grammar guarantees current_state.phi.has_value()
    current_state.training_references.filter(current_state.phi.value(), node.get_mode());
}

void ConcreteSemantics::visit(const ReturnNode &node) {
    return_value = current_state.posterior;
}

