#include "PrettyPrinter.h"
#include "ASTNode.h"
#include <iostream>
using namespace std;


void PrettyPrinter::printIndent() {
    for(int i = 0; i < indent; i++) {
        cout << "  ";
    }
}

PrettyPrinter::PrettyPrinter() {
    indent = 0;
}

void PrettyPrinter::visit(SequenceNode &node){
    for(int i = 0; i < node.get_num_children(); i++) {
        node.get_child(i)->accept(*this);
    }
}

void PrettyPrinter::visit(ITEImpurityNode &node){
    printIndent();
    cout << "if(impurity(T) = 0) {" << endl;
    indent++;
    node.get_then_child()->accept(*this);
    indent--;
    printIndent();
    cout << "} else {" << endl;
    indent++;
    node.get_else_child()->accept(*this);
    indent--;
    printIndent();
    cout << "}" << endl;
}

void PrettyPrinter::visit(ITEModelsNode &node){
    printIndent();
    cout << "if(x models phi) {" << endl;
    indent++;
    node.get_then_child()->accept(*this);
    indent--;
    printIndent();
    cout << "} else {" << endl;
    indent++;
    node.get_else_child()->accept(*this);
    indent--;
    printIndent();
    cout << "}" << endl;
}

void PrettyPrinter::visit(BestSplitNode &node){
    printIndent();
    cout << "phi <- bestsplit(T);" << endl;
}

void PrettyPrinter::visit(FilterNode &node){
    printIndent();
    cout << "T <- filter(T, " << (node.get_mode() ? "" : "not ") << "phi);" << endl;
}

void PrettyPrinter::visit(SummaryNode &node){
    printIndent();
    cout << "p <- summary(T);" << endl;
}

void PrettyPrinter::visit(ReturnNode &node){
    printIndent();
    cout << "return p;" << endl;
}
