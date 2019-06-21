#ifndef ASTNODE_H
#define ASTNODE_H


/**
 * The AST class hierarchy here looks like the following:
 * ASTNode (abstract)
 *   SequenceNode
 *   ITENode (no constructor)
 *      ITEImpurityNode
 *      ITEModelsNode
 *   BestSplitNode
 *   FilterNode
 *   SummaryNode
 *   ReturnNode
 *
 * It represents this simple Decision-Tree-Learning-Classification DSL:
 *        Statements S := S_1; S_2; ...; S_n
 *                      | if impurity(T) = 0 then S_1 else S_2
 *                      | if x models phi then S_1 else S_2
 *                      | phi <- bestsplit(T)
 *                      | T <- filter(T, phi)
 *                      | T <- filter(T, not phi)
 *                      | p <- summary(T)
 * Return Statements R := return p
 *           Program P := S; R
 * Note that x, T, phi, and p are not non-terminals---they are part of the syntax.
 * Indeed, x is fixed a priori, and the semantics of the program update a simple state
 * consisting of the (T, phi, p) triple.
 *
 * Additionally, this file includes an interface for AST traversal (Visitor).
 **/


class Visitor;


class ASTNode {
public:
    static ASTNode* buildTree(int depth); // XXX heap is never deallocated
    virtual void accept(Visitor &v) const = 0;
    //TODO implement destructor
};


class SequenceNode : public ASTNode {
private:
    ASTNode **children;
    int num_children;

public:
    SequenceNode(ASTNode **children, int num_children);
    void accept(Visitor &v) const;

    ASTNode** get_children() const { return children; }
    int get_num_children() const { return num_children; }
    ASTNode* get_child(int index) const { return children[index]; }
};


class ITENode : public ASTNode {
protected:
    ASTNode *then_child, *else_child;

public:
    const ASTNode* get_then_child() const { return then_child; }
    const ASTNode* get_else_child() const { return else_child; }
    void set_children(ASTNode *then_child, ASTNode *else_child);
};


class ITEImpurityNode : public ITENode {
public:
    ITEImpurityNode();
    void accept(Visitor &v) const;
};


class ITEModelsNode : public ITENode {
public:
    ITEModelsNode(bool populate);
    void accept(Visitor &v) const;
};


class BestSplitNode : public ASTNode {
public:
    BestSplitNode();
    void accept(Visitor &v) const;
};


class FilterNode : public ASTNode {
private:
    bool mode; // Determines whether or not phi is negated

public:
    FilterNode(bool mode);
    void accept(Visitor &v) const;

    bool get_mode() const { return mode; }
};


class SummaryNode : public ASTNode {
public:
    SummaryNode();
    void accept(Visitor &v) const;
};


class ReturnNode : public ASTNode {
public:
    ReturnNode();
    void accept(Visitor &v) const;
};


class Visitor {
public:
    virtual void visit(const SequenceNode &node) = 0;
    virtual void visit(const ITEImpurityNode &node) = 0;
    virtual void visit(const ITEModelsNode &node) = 0;
    virtual void visit(const BestSplitNode &node) = 0;
    virtual void visit(const FilterNode &node) = 0;
    virtual void visit(const SummaryNode &node) = 0;
    virtual void visit(const ReturnNode &node) = 0;
};


#endif
