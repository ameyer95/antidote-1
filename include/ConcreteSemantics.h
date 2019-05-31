#ifndef CONCRETESEMANTICS_H
#define CONCRETESEMANTICS_H

#include "ASTNode.h"
#include "DataSet.h"

// Everything is currently hard-coded for binary classification and boolean features.


//TODO ensure training_set etc are appropriately passed around by reference / as pointers
class ConcreteSemantics : public Visitor {
private:
    Input test_input;
    DataSet *training_set;
    double posterior;
    Predicate *phi;
    PredicateSet predicates;
    double return_value; // Largely a proxy for posterior
    bool halt; // Controls whether we've hit a return statement

public:
    ConcreteSemantics();

    double execute(Input test_input, DataSet *training_set, PredicateSet predicates, ASTNode *program);

    void visit(SequenceNode &node);
    void visit(ITEImpurityNode &node);
    void visit(ITEModelsNode &node);
    void visit(BestSplitNode &node);
    void visit(FilterNode &node);
    void visit(SummaryNode &node);
    void visit(ReturnNode &node);
};


#endif
