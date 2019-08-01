#include "PrettyPrinter.h"
#include "ASTNode.h"
#include <string>
#include <vector>
using namespace std;


void PrettyPrinter::printIndent() {
    for(int i = 0; i < indent; i++) {
        code_string += "  ";
    }
}

void PrettyPrinter::printLine(string s) {
    printIndent();
    code_string += s + "\n";
}

PrettyPrinter::PrettyPrinter() {
    reset();
}

void PrettyPrinter::reset() {
    indent = 0;
    code_string = "";
}

void PrettyPrinter::visit(const ProgramNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

void PrettyPrinter::visit(const SequenceNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

void PrettyPrinter::visit(const ITEImpurityNode &node) {
    printLine("if(impurity(T) = 0) {");
    indent++;
    node.get_left_child()->accept(*this);
    indent--;
    printLine("} else {");
    indent++;
    node.get_right_child()->accept(*this);
    indent--;
    printLine("}");
}

void PrettyPrinter::visit(const ITENoPhiNode &node) {
    printLine("if(phi = null) {");
    indent++;
    node.get_left_child()->accept(*this);
    indent--;
    printLine("} else {");
    indent++;
    node.get_right_child()->accept(*this);
    indent--;
    printLine("}");
}

void PrettyPrinter::visit(const BestSplitNode &node) {
    printLine("phi <- bestsplit(T);");
}

void PrettyPrinter::visit(const SummaryNode &node) {
    printLine("p <- summary(T);");
}

void PrettyPrinter::visit(const UsePhiSequenceNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

void PrettyPrinter::visit(const ITEModelsNode &node) {
    printLine("if(x models phi) {");
    indent++;
    node.get_left_child()->accept(*this);
    indent--;
    printLine("} else {");
    indent++;
    node.get_right_child()->accept(*this);
    indent--;
    printLine("}");
}

void PrettyPrinter::visit(const FilterNode &node) {
    string mode = (node.get_mode() ? "" : "not ");
    printLine("T <- filter(T, " + mode + "phi);");
}

void PrettyPrinter::visit(const ReturnNode &node) {
    printLine("return p;");
}
