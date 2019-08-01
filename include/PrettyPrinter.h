#ifndef PRETTYPRINTER_H
#define PRETTYPRINTER_H

#include "ASTNode.h"
#include <string>
using namespace std;


class PrettyPrinter : public ASTVisitor {
private:
    int indent;
    string code_string;

    void printIndent();
    void printLine(string s);

public:
    PrettyPrinter();

    void reset();
    string getString() { return code_string; }

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
