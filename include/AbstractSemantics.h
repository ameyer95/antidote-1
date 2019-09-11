#ifndef ABSTRACTSEMANTICS_H
#define ABSTRACTSEMANTICS_H

#include "ASTNode.h"
#include "StateDomain.h"
#include <vector>


/**
 * Very general abstract semantics class.
 *
 * Template parameters are:
 * E - state abstraction class
 * T - concrete training set class
 * P - concrete predicate class
 * D - concrete posterior distribution class
 *
 * The actual abstract domain over E is a member of the class.
 */
template <typename E, typename T, typename P, typename D>
class AbstractSemantics : public ASTVisitor {
private:
    E current_state;
    StateDomain<E,T,P,D> state_domain;
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

template <typename E, typename T, typename P, typename D>
void AbstractSemantics<E,T,P,D>::visit(const ProgramNode &node) {
    // TODO it's simple composition, but the starting point is awkward.
}

template <typename E, typename T, typename P, typename D>
void AbstractSemantics<E,T,P,D>::visit(const SequenceNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

template <typename E, typename T, typename P, typename D>
void AbstractSemantics<E,T,P,D>::visit(const ITEImpurityNode &node) {
    std::vector<E> joins;
    E pass_to_then, pass_to_else, backup;

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

template <typename E, typename T, typename P, typename D>
void AbstractSemantics<E,T,P,D>::visit(const ITENoPhiNode &node) {
    std::vector<E> joins;
    E pass_to_then, pass_to_else, backup;

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

template <typename E, typename T, typename P, typename D>
void AbstractSemantics<E,T,P,D>::visit(const BestSplitNode &node) {
    current_state = state_domain.applyBestSplit(current_state);
}

template <typename E, typename T, typename P, typename D>
void AbstractSemantics<E,T,P,D>::visit(const SummaryNode &node) {
    current_state = state_domain.applySummary(current_state);
}

template <typename E, typename T, typename P, typename D>
void AbstractSemantics<E,T,P,D>::visit(const UsePhiSequenceNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

template <typename E, typename T, typename P, typename D>
void AbstractSemantics<E,T,P,D>::visit(const ITEModelsNode &node) {
    std::vector<E> joins;
    E pass_to_then, pass_to_else, backup;

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

template <typename E, typename T, typename P, typename D>
void AbstractSemantics<E,T,P,D>::visit(const FilterNode &node) {
    if(node.get_mode()) {
        current_state = state_domain.applyFilter(current_state);
    } else {
        current_state = state_domain.applyFilterNegated(current_state);
    }
}

template <typename E, typename T, typename P, typename D>
void AbstractSemantics<E,T,P,D>::visit(const ReturnNode &node) {
    // TODO it should just involve storing a value
}

#endif
