#ifndef CONCRETESEMANTICS_H
#define CONCRETESEMANTICS_H

#include "ASTNode.h"
#include "concrete_common.h"
#include "data_common.h"
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

    const DataRow& getRow(int row_index) const { return (*data)[row_index]; }
    bool classificationBit(int row_index) { return (*data)[row_index].second; }
    int countOnes();
    pair<pair<int, int>, pair<int, int>> splitCounts(const BitVectorPredicate *phi);

public:
    // Does not handle deallocation
    BooleanDataSet(DataReferences<DataRow> *data) { this->data = data; }

    bool isPure();
    void filter(const BitVectorPredicate &phi, bool mode);
    double summary();
    const BitVectorPredicate* bestSplit(const vector<BitVectorPredicate> *predicates);
};


//TODO ensure training_set etc are appropriately passed around by reference / as pointers
class ConcreteSemantics : public Visitor {
private:
    Input test_input;
    BooleanDataSet *training_set;
    double posterior;
    const BitVectorPredicate *phi;
    const vector<BitVectorPredicate> *predicates;
    double return_value; // Largely a proxy for posterior
    bool halt; // Controls whether we've hit a return statement

public:
    ConcreteSemantics();
    // This class doesn't do any dynamic allocation and accordingly does not handle any deallocation

    double execute(const Input test_input, BooleanDataSet *training_set, const vector<BitVectorPredicate> *predicates, const ASTNode *program);

    void visit(const SequenceNode &node);
    void visit(const ITEImpurityNode &node);
    void visit(const ITEModelsNode &node);
    void visit(const BestSplitNode &node);
    void visit(const FilterNode &node);
    void visit(const SummaryNode &node);
    void visit(const ReturnNode &node);
};


#endif
