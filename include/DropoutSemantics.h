#ifndef DROPOUTSEMANTICS_H
#define DROPOUTSEMANTICS_H

#include "ASTNode.h"
#include "ConcreteSemantics.h" // For hard-coded types: Input, DataRow, BitVectorPredicate
#include "data_common.h"
#include "Interval.h"
#include <utility>
#include <vector>

// Everything is currently hard-coded avoiding generality

typedef std::vector<BitVectorPredicate> PredicateSet;
typedef std::vector<BitVectorPredicate*> PredicatePointers;

struct DropoutCounts {
    int pos, neg, num_dropout;
};

class DropoutSet {
private:
    DataReferences<DataRow> data;
    int num_dropout;
    bool bot_flag;

    const DataRow& getRow(int row_index) const { return data[row_index]; }
    bool classificationBit(int row_index) { return data[row_index].second; }
    Interval<int> countOnes();

    std::pair<DropoutCounts, DropoutCounts> splitCounts(const BitVectorPredicate &phi);

public:
    DropoutSet() { bot_flag = true; }
    // Does not handle deallocation
    DropoutSet(DataReferences<DataRow> data, int num_dropout) { this->data = data; this->num_dropout = num_dropout; bot_flag = false; }

    DropoutSet* pureSets(bool classification); // Returns NULL as a bottom element
    PredicatePointers bestSplit(const PredicateSet *predicates);
    void filter(const BitVectorPredicate &phi, bool mode);
    Interval<double> summary();

    Interval<int> size();

    bool isBottom() const { return bot_flag; }
    
    static DropoutSet join(const DropoutSet &e1, const DropoutSet &e2);
    static DropoutSet join(const vector<DropoutSet> &elements);
};


class AbstractState {
public:
    DropoutSet training_set;
    PredicatePointers phis;
    Interval<double> posterior;
    bool bot_flag;

    AbstractState() { bot_flag = true; }

    static AbstractState join(const AbstractState &e1, const AbstractState &e2);
    static AbstractState join(const vector<AbstractState> &elements);
};


class DropoutSemantics : public ASTVisitor {
private:
    AbstractState current_state;
    Input test_input;
    const PredicateSet *predicates;
    Interval<double> return_value; // Largely a proxy for posterior

public:
    DropoutSemantics();

    Interval<double> execute(const Input test_input, DropoutSet *training_set, const PredicateSet *predicates, const ProgramNode *program);

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
