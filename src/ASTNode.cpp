#include "ASTNode.h"
#include <vector>
using namespace std;


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
 * Various constructors and destructors
 **/

// Because ASTNode::~ASTNode() was declared pure virtual,
// providing this is necessary AND still requires subclasses to implement a destructor.
ASTNode::~ASTNode() {}

FilterNode::FilterNode(bool mode) {
    this->mode = mode;
}


/**
 * Visitor accept methods
 **/

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
