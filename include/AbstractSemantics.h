#ifndef ABSTRACTSEMANTICS_H
#define ABSTRACTSEMANTICS_H

#include "ASTNode.h"
#include "StateDomain.h"
#include <vector>


/**
 * Very general abstract semantics class.
 *
 * Template parameter A is the state abstraction class.
 * L is the abstract domain class---should be StateDomain<A>.
 */
template <typename L, typename A>
class AbstractSemantics : public ASTVisitor {
private:
    A current_state;
    L state_domain;
public:
    void visit(const ProgramNode &node);
    void visit(const SequenceNode &node);
    void visit(const ITEImpurityNode &node);
    void visit(const ITENoPhiNode &node);
    void visit(const BestSplitNode &node);
    void visit(const SummaryNode &node);
    void visit(const UsePhiSequenceNode &node);
    void visit(const ITEModelsNode &node);
    void visit(const FilterNode &node);
    void visit(const ReturnNode &node);
};


/**
 * Because templates + incremental compilation = a nightmare,
 * we commit the sin of including all these template function bodies in the header, below.
 * They are, for the most part, simple methods that delegate responsibilities
 * to the state abstract domain instance.
 */

template <typename L, typename A>
void AbstractSemantics<L,A>::visit(const ProgramNode &node) {
    // TODO it's simple composition, but the starting point is awkward.
}

template <typename L, typename A>
void AbstractSemantics<L,A>::visit(const SequenceNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

template <typename L, typename A>
void AbstractSemantics<L,A>::visit(const ITEImpurityNode &node) {
    std::vector<A> joins;
    A pass_to_then, pass_to_else, backup;

    pass_to_then = state_domain.meetImpurityEqualsZero(current_state);
    if(!pass_to_then.isBottomElement()) {
        backup = current_state;
        node.get_left_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    pass_to_else = state_domain.meetImpurityNotEqualsZero(current_state);
    if(!pass_to_else.isBottomElement()) {
        backup = current_state;
        node.get_right_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    current_state = state_domain.join(joins);
}

template <typename L, typename A>
void AbstractSemantics<L,A>::visit(const ITENoPhiNode &node) {
    std::vector<A> joins;
    A pass_to_then, pass_to_else, backup;

    pass_to_then = state_domain.meetPhiIsBottom(current_state);
    if(!pass_to_then.isBottomElement()) {
        backup = current_state;
        node.get_left_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    pass_to_else = state_domain.meetPhiIsNotBottom(current_state);
    if(!pass_to_else.isBottomElement()) {
        backup = current_state;
        node.get_right_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    current_state = state_domain.join(joins);
}

template <typename L, typename A>
void AbstractSemantics<L,A>::visit(const BestSplitNode &node) {
    current_state = state_domain.applyBestSplit(current_state);
}

template <typename L, typename A>
void AbstractSemantics<L,A>::visit(const SummaryNode &node) {
    current_state = state_domain.applySummary(current_state);
}

template <typename L, typename A>
void AbstractSemantics<L,A>::visit(const UsePhiSequenceNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

template <typename L, typename A>
void AbstractSemantics<L,A>::visit(const ITEModelsNode &node) {
    std::vector<A> joins;
    A pass_to_then, pass_to_else, backup;

    pass_to_then = state_domain.meetXModelsPhi(current_state);
    if(!pass_to_then.isBottomElement()) {
        backup = current_state;
        node.get_left_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    pass_to_else = state_domain.meetXNotModelsPhi(current_state);
    if(!pass_to_else.isBottomElement()) {
        backup = current_state;
        node.get_right_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    current_state = state_domain.join(joins);
}

template <typename L, typename A>
void AbstractSemantics<L,A>::visit(const FilterNode &node) {
    if(node.get_mode()) {
        current_state = state_domain.applyFilter(current_state);
    } else {
        current_state = state_domain.applyFilterNegated(current_state);
    }
}

template <typename L, typename A>
void AbstractSemantics<L,A>::visit(const ReturnNode &node) {
    // TODO it should just involve storing a value
}

#endif
