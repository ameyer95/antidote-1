#ifndef ABSTRACTSEMANTICSTEMPLATE_H
#define ABSTRACTSEMANTICSTEMPLATE_H

#include "ASTNode.h"
#include "Feature.hpp"
#include "StateDomainTemplate.hpp"


/**
 * Very general abstract semantics class.
 *
 * Template parameter A is the state abstraction class.
 * We keep track of a StateDomainTemplate<A> pointer to manipulate the A elements.
 *
 * The main callable function, execute, returns the whole abstract state
 * that reaches the return statement.
 */
template <typename A>
class AbstractSemanticsTemplate : public ASTVisitor {
private:
    A current_state;
    FeatureVector test_input;
    const StateDomainTemplate<A> *state_domain;

public:
    AbstractSemanticsTemplate(const StateDomainTemplate<A> *state_domain) { this->state_domain = state_domain; }

    A execute(const FeatureVector &test_input, A initial_state, const ProgramNode *program);

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


#endif
