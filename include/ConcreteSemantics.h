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
    bool evaluate(const Input &input) { return input[index]; }
};


class BooleanDataSet : public DataSetInterface<BitVectorPredicate, vector<BitVectorPredicate>, double> {
private:
    DataReferences<DataRow> *data;

public:
    //TODO implement these
    bool isPure();
    void filter(const BitVectorPredicate &phi, bool mode);
    double summary();
    BitVectorPredicate* bestSplit(vector<BitVectorPredicate> *predicates);

    const DataRow& getRow(int index) { return (*data)[index]; }
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
