#include "ASTNode.h"
#include <iostream>
using namespace std;


/**
 * The program-building functions
 **/

ASTNode* buildTreeUnit(ASTNode* base) {
    ITEImpurityNode *impurity;
    SequenceNode *sequence;
    ASTNode **sequence_children = new ASTNode*[3];
    ITEModelsNode *models;

    models = new ITEModelsNode(true);

    sequence_children[0] = new BestSplitNode();
    sequence_children[1] = models;
    sequence_children[2] = base;

    sequence = new SequenceNode(sequence_children, 3);
    
    impurity = new ITEImpurityNode();
    impurity->set_children(new SummaryNode(), sequence);

    return impurity;
}

ASTNode* ASTNode::buildTree(int depth) {
    ASTNode **sequence_children = new ASTNode*[2];
    ASTNode *current = new SummaryNode();
    for(int i = 0; i < depth; i++) {
        current = buildTreeUnit(current);
    }
    sequence_children[0] = current;
    sequence_children[1] = new ReturnNode();
    return new SequenceNode(sequence_children, 2);
}


/**
 * Various constructors
 **/

SequenceNode::SequenceNode(ASTNode **children, int num_children) {
    this->children = children;
    this->num_children = num_children;
}

ITEImpurityNode::ITEImpurityNode() {}

// When populate is true, we make the branches the obvious choices of filters
ITEModelsNode::ITEModelsNode(bool populate) {
    if(populate) {
        this->set_children(new FilterNode(true), new FilterNode(false));
    }
}

BestSplitNode::BestSplitNode() {}

FilterNode::FilterNode(bool mode) {
    this->mode = mode;
}

SummaryNode::SummaryNode() {}

ReturnNode::ReturnNode() {}


/**
 * Visitor accept methods
 **/

void SequenceNode::accept(Visitor &v) const { v.visit(*this); }
void ITEImpurityNode::accept(Visitor &v) const { v.visit(*this); }
void ITEModelsNode::accept(Visitor &v) const { v.visit(*this); }
void BestSplitNode::accept(Visitor &v) const { v.visit(*this); }
void FilterNode::accept(Visitor &v) const { v.visit(*this); }
void SummaryNode::accept(Visitor &v) const { v.visit(*this); }
void ReturnNode::accept(Visitor &v) const { v.visit(*this); }


/**
 * Other
 **/

void ITENode::set_children(ASTNode *then_child, ASTNode *else_child) {
    this->then_child = then_child;
    this->else_child = else_child;
}
