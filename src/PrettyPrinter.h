#ifndef PRETTYPRINTER_H
#define PRETTYPRINTER_H

#include "ASTNode.h"


class PrettyPrinter : public Visitor {
private:
    int indent;

    void printIndent();

public:
    PrettyPrinter();

    void visit(SequenceNode &node);
    void visit(ITEImpurityNode &node);
    void visit(ITEModelsNode &node);
    void visit(BestSplitNode &node);
    void visit(FilterNode &node);
    void visit(SummaryNode &node);
    void visit(ReturnNode &node);
};


#endif
