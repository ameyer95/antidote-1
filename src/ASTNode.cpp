#include "ASTNode.h"

// We'll have the ASTVisitor subclass output entering/exiting information
// (by modifying the ASTNode half of dual-dispatch)
// when -DDEBUG is given as a g++ argument
#ifdef DEBUG
#include <iostream>
#endif

/**
 * The program-building functions
 **/

StatementProduction* buildTreeUnit(StatementProduction *base) {
    ITEImpurityNode *impurity;
    SequenceNode *impurity_else;
    ITENoPhiNode *nophi;
    UsePhiSequenceNode *nophi_else;
    ITEModelsNode *models;

    models = new ITEModelsNode(new FilterNode(true), new FilterNode(false));
    nophi_else = new UsePhiSequenceNode(models, base);
    nophi = new ITENoPhiNode(new SummaryNode(), nophi_else);
    impurity_else = new SequenceNode(new BestSplitNode(), nophi);
    impurity = new ITEImpurityNode(new SummaryNode(), impurity_else);

    return impurity;
}

ProgramNode* buildTree(int depth) {
    StatementProduction *current = new SummaryNode();
    for(int i = 0; i < depth; i++) {
        current = buildTreeUnit(current);
    }
    return new ProgramNode(current, new ReturnNode());
}


/**
 * Various member functions
 **/

// Because ASTNode::~ASTNode() was declared pure virtual
// (to enforce subclasses to implement destructors),
// we actually still need to provide a definition for the abstract base case.
ASTNode::~ASTNode() {}


/**
 * Visitor accept methods
 **/

// As stated at the beginning of the file,
// We'll include entering/exiting information when debugging
#ifndef DEBUG
void ProgramNode::accept(ASTVisitor &v) const { v.visit(*this); }
void SequenceNode::accept(ASTVisitor &v) const { v.visit(*this); }
void ITEImpurityNode::accept(ASTVisitor &v) const { v.visit(*this); }
void ITENoPhiNode::accept(ASTVisitor &v) const { v.visit(*this); }
void BestSplitNode::accept(ASTVisitor &v) const { v.visit(*this); }
void SummaryNode::accept(ASTVisitor &v) const { v.visit(*this); }
void UsePhiSequenceNode::accept(ASTVisitor &v) const { v.visit(*this); }
void ITEModelsNode::accept(ASTVisitor &v) const { v.visit(*this); }
void FilterNode::accept(ASTVisitor &v) const { v.visit(*this); }
void ReturnNode::accept(ASTVisitor &v) const { v.visit(*this); }
#else
void ProgramNode::accept(ASTVisitor &v) const {
    std::cout << "Entering ProgramNode" << std::endl;
    v.visit(*this);
    std::cout << "Exiting ProgramNode" << std::endl;
}
void SequenceNode::accept(ASTVisitor &v) const {
    std::cout << "Entering SequenceNode" << std::endl;
    v.visit(*this);
    std::cout << "Exiting SequenceNode" << std::endl;
}
void ITEImpurityNode::accept(ASTVisitor &v) const {
    std::cout << "Entering ITEImpurityNode" << std::endl;
    v.visit(*this);
    std::cout << "Exiting ITEImpurityNode" << std::endl;
}
void ITENoPhiNode::accept(ASTVisitor &v) const {
    std::cout << "Entering ITENoPhiNode" << std::endl;
    v.visit(*this);
    std::cout << "Exiting ITENoPhiNode" << std::endl;
}
void BestSplitNode::accept(ASTVisitor &v) const {
    std::cout << "Entering BestSplitNode" << std::endl;
    v.visit(*this);
    std::cout << "Exiting BestSplitNode" << std::endl;
}
void SummaryNode::accept(ASTVisitor &v) const {
    std::cout << "Entering SummaryNode" << std::endl;
    v.visit(*this);
    std::cout << "Exiting SummaryNode" << std::endl;
}
void UsePhiSequenceNode::accept(ASTVisitor &v) const {
    std::cout << "Entering UsePhiSequenceNode" << std::endl;
    v.visit(*this);
    std::cout << "Exiting UsePhiSequenceNode" << std::endl;
}
void ITEModelsNode::accept(ASTVisitor &v) const {
    std::cout << "Entering ITEModelsNode" << std::endl;
    v.visit(*this);
    std::cout << "Exiting ITEModelsNode" << std::endl;
}
void FilterNode::accept(ASTVisitor &v) const {
    std::cout << "Entering FilterNode" << std::endl;
    v.visit(*this);
    std::cout << "Exiting FilterNode" << std::endl;
}
void ReturnNode::accept(ASTVisitor &v) const {
    std::cout << "Entering ReturnNode" << std::endl;
    v.visit(*this);
    std::cout << "Exiting ReturnNode" << std::endl;
}
#endif
