#ifndef DATASET_H
#define DATASET_H

#include <vector>
#include <utility> // For pair class
using namespace std;


// Everything is currently hard-coded for binary classification and boolean features.

// TODO remove this file, incorporating its details into ConcreteSemantics.h, ConcreteCommon.h, etc

class Predicate;

typedef vector<bool> Input;
typedef pair<Input, bool> DataRow;


class DataSet {
private:
    vector<DataRow> *data;

public:
    DataSet(bool *bits, int num_bits, int row_size);
    DataSet(vector<DataRow> *data);

    DataRow getRow(int row_index) { return (*data)[row_index]; }
    int size() { return data->size(); }

    bool classificationBit(int row_index) { return (*data)[row_index].second; }
    int countOnes();

    bool isPure();
    void filter(Predicate &phi, bool mode);
    double summary();

    double impurity();
    pair<DataSet*, DataSet*> split(Predicate &phi);
};


class Predicate {
private:
    int bit_index;

public:
    Predicate(int bit_index) { this->bit_index = bit_index; }
    bool evaluate(Input x);
};


class PredicateSet {
private:
    vector<Predicate> *predicates;

    double informationGain(Predicate &phi, DataSet &training_set);

public:
    PredicateSet(vector<Predicate> *predicates) { this->predicates = predicates; }
    Predicate* bestSplit(DataSet &training_set);
};


#endif
