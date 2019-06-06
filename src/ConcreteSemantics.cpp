#include "ConcreteSemantics.h"
#include "DataSet.h"


ConcreteSemantics::ConcreteSemantics() {
    //TODO
}

double ConcreteSemantics::execute(Input test_input, DataSet *training_set, PredicateSet *predicates, ASTNode *program) {
    this->test_input = test_input;
    this->training_set = training_set;
    this->predicates = predicates;
    halt = false;
    program->accept(*this);
    return return_value;
}

void ConcreteSemantics::visit(SequenceNode &node) {
    if(halt) return;
    for(int i = 0; i < node.get_num_children(); i++) {
        node.get_child(i)->accept(*this);
    }
}

void ConcreteSemantics::visit(ITEImpurityNode &node) {
    if(halt) return;
    if(training_set->isPure()) {
        node.get_then_child()->accept(*this);
    } else {
        node.get_else_child()->accept(*this);
    }
}

void ConcreteSemantics::visit(ITEModelsNode &node) {
    if(halt) return;
    if(phi->evaluate(test_input)) {
        node.get_then_child()->accept(*this);
    } else {
        node.get_else_child()->accept(*this);
    }
}

void ConcreteSemantics::visit(BestSplitNode &node) {
    if(halt) return;
    phi = predicates->bestSplit(*training_set);
}

void ConcreteSemantics::visit(FilterNode &node) {
    if(halt) return;
    training_set->filter(*phi, node.get_mode());
}

void ConcreteSemantics::visit(SummaryNode &node) {
    if(halt) return;
    posterior = training_set->summary();
}

void ConcreteSemantics::visit(ReturnNode &node) {
    if(halt) return;
    return_value = posterior;
    halt = true;
}
