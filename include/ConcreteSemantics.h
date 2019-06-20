#ifndef CONCRETESEMANTICS_H
#define CONCRETESEMANTICS_H

#include "ASTNode.h"
#include "ConcreteCommon.h"
#include "DataCommon.h"
#include <utility>
#include <vector>
using namespace std;

// Everything is currently hard-coded for binary classification and boolean features.
typedef vector<bool> Input;
typedef pair<Input, bool> DataRow;


class BitVectorPredicate : public Predicate<Input> {
private:
    int index;

public:
    BitVectorPredicate(int index) { this->index = index; }
    const bool evaluate(const Input &input) const { return input[index]; }
};


class BooleanDataSet : public DataSetInterface<BitVectorPredicate, vector<BitVectorPredicate>, double> {
private:
    DataReferences<DataRow> *data;

    bool classificationBit(int row_index) { return (*data)[row_index].second; }
    int countOnes();
    pair<pair<int, int>, pair<int, int>> splitCounts(BitVectorPredicate *phi);

public:
    bool isPure();
    void filter(const BitVectorPredicate &phi, bool mode);
    double summary();
    BitVectorPredicate* bestSplit(vector<BitVectorPredicate> *predicates);

    const DataRow& getRow(int row_index) const { return (*data)[row_index]; }
};


//TODO ensure training_set etc are appropriately passed around by reference / as pointers
class ConcreteSemantics : public Visitor {
private:
    Input test_input;
    BooleanDataSet *training_set;
    double posterior;
    BitVectorPredicate *phi;
    vector<BitVectorPredicate> *predicates;
    double return_value; // Largely a proxy for posterior
    bool halt; // Controls whether we've hit a return statement

public:
    ConcreteSemantics();

    double execute(Input test_input, BooleanDataSet *training_set, vector<BitVectorPredicate> *predicates, ASTNode *program);

    void visit(SequenceNode &node);
    void visit(ITEImpurityNode &node);
    void visit(ITEModelsNode &node);
    void visit(BestSplitNode &node);
    void visit(FilterNode &node);
    void visit(SummaryNode &node);
    void visit(ReturnNode &node);
};


#endif
