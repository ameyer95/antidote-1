#include "ASTNode.h"
#include <vector>
using namespace std;


/**
 * The program-building functions
 **/

ASTNode* buildTreeUnit(ASTNode* base) {
    ITEImpurityNode *impurity;
    SequenceNode *sequence;
    vector<ASTNode*> sequence_children(3);
    ITEModelsNode *models;

    models = new ITEModelsNode(new FilterNode(true), new FilterNode(false));

    sequence_children[0] = new BestSplitNode();
    sequence_children[1] = models;
    sequence_children[2] = base;

    sequence = new SequenceNode(sequence_children);
    
    impurity = new ITEImpurityNode(new SummaryNode, sequence);

    return impurity;
}

ASTNode* ASTNode::buildTree(int depth) {
    vector<ASTNode*> sequence_children(2);
    ASTNode *current = new SummaryNode();
    for(int i = 0; i < depth; i++) {
        current = buildTreeUnit(current);
    }
    sequence_children[0] = current;
    sequence_children[1] = new ReturnNode();
    return new SequenceNode(sequence_children);
}


/**
 * Various constructors
 **/

SequenceNode::SequenceNode(const vector<ASTNode*> &children) {
    this->children = children;
}

ITENode::ITENode(const ASTNode *then_child, const ASTNode *else_child) {
    this->then_child = then_child;
    this->else_child = else_child;
}

FilterNode::FilterNode(bool mode) {
    this->mode = mode;
}


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
