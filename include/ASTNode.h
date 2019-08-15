#ifndef ASTNODE_H
#define ASTNODE_H

/**
 * The AST class hierarchy here is slightly involved;
 * we use the C++ type system to enforce the grammar productions.
 * Additionally, the production rules generally involve either 0 or 2 non-terminals,
 * the latter of which all thus subclass such a template.
 * It's shown below:
 *
 * ASTNode (abstract)                  PairNodeTemplate (template)
 *   ProgramProduction (abstract)          |
 *     ProgramNode ------------------------|
 *   StatementProduction (abstract)        |
 *     SequenceNode -----------------------|
 *     ITEImpurityNode --------------------|
 *     ITENoPhiNode -----------------------|
 *     BestSplitNode                       |
 *     SummaryNode                         |
 *   UsePhiStatementProduction (abstract)  |
 *     UsePhiSequenceNode -----------------|
 *     ITEModelsNode ----------------------|
 *     FilterNode
 *   ReturnStatementProduction (abstract)
 *     ReturnNode
 *
 * It represents this simple Decision-Tree-Learning-Classification DSL:
 *            Program P := S ; R
 *         Statements S := S_1 ; S_2
 *                       | if impurity(T) = 0 then S_1 else S_2
 *                       | if phi == bot then S else U
 *                       | phi <- bestsplit(T)
 *                       | p <- summary(T)
 * Use-phi Statements U := U ; S
 *                       | if x models phi then U_1 else U_2
 *                       | T <- filter(T, phi)
 *                       | T <- filter(T, not phi)
 *  Return Statements R := return p
 *
 * Note that while P, S, R, and U are non-terminals,
 * x, T, phi, and p are not---they are part of the syntax.
 * Indeed, x is fixed a priori, and the semantics of the program update a simple state
 * consisting of the (T, phi, p) triple.
 *
 * Additionally, this file includes:
 *   1) an interface for AST traversal (ASTVisitor), and
 *   2) a method for building tree programs of a specified depth (buildTree).
 **/


/**
 * The main AST abstract base class
 */

// Forward declare the visitor since the ASTNode subclasses need accept(ASTVisitor...) methods
class ASTVisitor;


class ASTNode {
public:
    virtual ~ASTNode() = 0;
    virtual void accept(ASTVisitor &v) const = 0;
};


/**
 * The various grammar production types
 */

class ProgramProduction : public ASTNode {};
class StatementProduction : public ASTNode {};
class UsePhiStatementProduction : public ASTNode {};
class ReturnStatementProduction : public ASTNode {};


/**
 * Generic ASTNode templates since, e.g., there are multiple if-then-else productions.
 */


template <typename L, typename R>
class PairNodeTemplate {
protected:
    const L *left_child;
    const R *right_child;

public:
    PairNodeTemplate(const L *left_child, const R *right_child) {
        this->left_child = left_child;
        this->right_child = right_child;
    }
    ~PairNodeTemplate() {
        delete left_child;
        delete right_child;
    }

    const L* get_left_child() const { return left_child; }
    const R* get_right_child() const { return right_child; }
};


/**
 * ProgramProduction grammar productions
 */


class ProgramNode : public ProgramProduction, public PairNodeTemplate<StatementProduction, ReturnStatementProduction> {
public:
    ProgramNode(StatementProduction *left_child, ReturnStatementProduction *right_child)
        : PairNodeTemplate<StatementProduction, ReturnStatementProduction>(left_child, right_child) {};
    void accept(ASTVisitor &v) const;
};


/**
 * StatementProduction grammar productions
 */


class SequenceNode : public StatementProduction, public PairNodeTemplate<StatementProduction, StatementProduction> {
public:
    SequenceNode(const StatementProduction *left_child, const StatementProduction *right_child)
        : PairNodeTemplate<StatementProduction, StatementProduction>(left_child, right_child) {};
    void accept(ASTVisitor &v) const;
};


class ITEImpurityNode : public StatementProduction, public PairNodeTemplate<StatementProduction, StatementProduction> {
public:
    ITEImpurityNode(const StatementProduction *left_child, const StatementProduction *right_child)
        : PairNodeTemplate<StatementProduction, StatementProduction>(left_child, right_child) {};
    void accept(ASTVisitor &v) const;
};


class ITENoPhiNode : public StatementProduction, public PairNodeTemplate<StatementProduction, UsePhiStatementProduction> {
public:
    ITENoPhiNode(const StatementProduction *left_child, const UsePhiStatementProduction *right_child)
        : PairNodeTemplate<StatementProduction, UsePhiStatementProduction>(left_child, right_child) {};
    void accept(ASTVisitor &v) const;
};


class BestSplitNode : public StatementProduction {
public:
    BestSplitNode() {};
    ~BestSplitNode() {};
    void accept(ASTVisitor &v) const;
};


class SummaryNode : public StatementProduction {
public:
    SummaryNode() {};
    ~SummaryNode() {};
    void accept(ASTVisitor &v) const;
};


/**
 * UsePhiStatementProduction grammar productions
 */


class UsePhiSequenceNode : public UsePhiStatementProduction, public PairNodeTemplate<UsePhiStatementProduction, StatementProduction> {
public:
    UsePhiSequenceNode(const UsePhiStatementProduction *left_child, const StatementProduction *right_child)
        : PairNodeTemplate<UsePhiStatementProduction, StatementProduction>(left_child, right_child) {};
    void accept(ASTVisitor &v) const;
};


class ITEModelsNode : public UsePhiStatementProduction, public PairNodeTemplate<UsePhiStatementProduction, UsePhiStatementProduction> {
public:
    ITEModelsNode(const UsePhiStatementProduction *left_child, const UsePhiStatementProduction *right_child)
        : PairNodeTemplate<UsePhiStatementProduction, UsePhiStatementProduction>(left_child, right_child) {};
    void accept(ASTVisitor &v) const;
};


class FilterNode : public UsePhiStatementProduction {
private:
    bool mode; // Determines whether or not phi is negated

public:
    FilterNode(bool mode) { this->mode = mode; }
    ~FilterNode() {};
    void accept(ASTVisitor &v) const;

    bool get_mode() const { return mode; }
};


/**
 * ReturnStatementProduction grammar productions
 */


class ReturnNode : public ReturnStatementProduction {
public:
    ReturnNode() {};
    ~ReturnNode() {};
    void accept(ASTVisitor &v) const;
};


/**
 * The actual visitor for AST traversal
 */


class ASTVisitor {
public:
    virtual void visit(const ProgramNode &node) = 0;
    virtual void visit(const SequenceNode &node) = 0;
    virtual void visit(const ITEImpurityNode &node) = 0;
    virtual void visit(const ITENoPhiNode &node) = 0;
    virtual void visit(const BestSplitNode &node) = 0;
    virtual void visit(const SummaryNode &node) = 0;
    virtual void visit(const UsePhiSequenceNode &node) = 0;
    virtual void visit(const ITEModelsNode &node) = 0;
    virtual void visit(const FilterNode &node) = 0;
    virtual void visit(const ReturnNode &node) = 0;
};


/**
 * The tree-building function
 */

ProgramNode* buildTree(int depth);

#endif
