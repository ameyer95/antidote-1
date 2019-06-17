#ifndef PRETTYPRINTER_H
#define PRETTYPRINTER_H

#include "ASTNode.h"
#include <string>
using namespace std;


class PrettyPrinter : public Visitor {
private:
    int indent;
    string code_string;

    void printIndent();
    void printLine(string s);

public:
    PrettyPrinter();

    void reset();
    string getString() { return code_string; }

    void visit(SequenceNode &node);
    void visit(ITEImpurityNode &node);
    void visit(ITEModelsNode &node);
    void visit(BestSplitNode &node);
    void visit(FilterNode &node);
    void visit(SummaryNode &node);
    void visit(ReturnNode &node);
};


#endif
