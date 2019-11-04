#include "AbstractSemanticsTemplate.h"
#include "AbstractSemanticsInstantiations.hpp" // Allows us to generate code
#include "Feature.hpp"
#include <vector>

template <typename A>
A AbstractSemanticsTemplate<A>::execute(const FeatureVector &test_input, A initial_state, const ProgramNode *program) {
    current_state = initial_state;
    this->test_input = test_input;
    program->accept(*this);
    return current_state;
}

template <typename A>
void AbstractSemanticsTemplate<A>::visit(const ProgramNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

template <typename A>
void AbstractSemanticsTemplate<A>::visit(const SequenceNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

template <typename A>
void AbstractSemanticsTemplate<A>::visit(const ITEImpurityNode &node) {
    std::vector<A> joins;
    A pass_to_then, pass_to_else, backup;

    pass_to_then = state_domain->meetImpurityEqualsZero(current_state);
    if(!state_domain->isBottomElement(pass_to_then)) {
        backup = current_state;
        current_state = pass_to_then;
        node.get_left_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    pass_to_else = state_domain->meetImpurityNotEqualsZero(current_state);
    if(!state_domain->isBottomElement(pass_to_else)) {
        backup = current_state;
        current_state = pass_to_else;
        node.get_right_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    current_state = state_domain->join(joins);
}

template <typename A>
void AbstractSemanticsTemplate<A>::visit(const ITENoPhiNode &node) {
    std::vector<A> joins;
    A pass_to_then, pass_to_else, backup;

    pass_to_then = state_domain->meetPhiIsBottom(current_state);
    if(!state_domain->isBottomElement(pass_to_then)) {
        backup = current_state;
        current_state = pass_to_then;
        node.get_left_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    pass_to_else = state_domain->meetPhiIsNotBottom(current_state);
    if(!state_domain->isBottomElement(pass_to_else)) {
        backup = current_state;
        current_state = pass_to_else;
        node.get_right_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    current_state = state_domain->join(joins);
}

template <typename A>
void AbstractSemanticsTemplate<A>::visit(const BestSplitNode &node) {
    current_state = state_domain->applyBestSplit(current_state);
}

template <typename A>
void AbstractSemanticsTemplate<A>::visit(const SummaryNode &node) {
    current_state = state_domain->applySummary(current_state);
}

template <typename A>
void AbstractSemanticsTemplate<A>::visit(const UsePhiSequenceNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

template <typename A>
void AbstractSemanticsTemplate<A>::visit(const ITEModelsNode &node) {
    std::vector<A> joins;
    A pass_to_then, pass_to_else, backup;

    pass_to_then = state_domain->meetXModelsPhi(current_state, test_input);
    if(!state_domain->isBottomElement(pass_to_then)) {
        backup = current_state;
        current_state = pass_to_then;
        node.get_left_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    pass_to_else = state_domain->meetXNotModelsPhi(current_state, test_input);
    if(!state_domain->isBottomElement(pass_to_else)) {
        backup = current_state;
        current_state = pass_to_else;
        node.get_right_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    current_state = state_domain->join(joins);
}

template <typename A>
void AbstractSemanticsTemplate<A>::visit(const FilterNode &node) {
    if(node.get_mode()) {
        current_state = state_domain->applyFilter(current_state);
    } else {
        current_state = state_domain->applyFilterNegated(current_state);
    }
}

template <typename A>
void AbstractSemanticsTemplate<A>::visit(const ReturnNode &node) {
    // Does not acutally have to do anything
    // since execute returns the whole abstract state that reaches here
    // (and the grammar enforces a single terminal return statement).
}
