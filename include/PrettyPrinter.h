#ifndef PRETTYPRINTER_H
#define PRETTYPRINTER_H

#include "ASTNode.h"
#include <string>


class PrettyPrinter : public ASTVisitor {
private:
    int indent;
    std::string code_string;

    void printIndent();
    void printLine(std::string s);

public:
    PrettyPrinter();

    void reset();
    std::string getString() { return code_string; }

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
