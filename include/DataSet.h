#ifndef DATASET_H
#define DATASET_H

#include <vector>
#include <utility> // For pair class
using namespace std;


// Everything is currently hard-coded for binary classification and boolean features.

class Predicate;

typedef vector<bool> Input;
typedef pair<Input, bool> DataRow;


class DataSet {
private:
    vector<DataRow> data;

public:
    DataSet(bool *bits, int num_bits, int row_size);

    DataRow getRow(int row_index) { return data[row_index]; }
    int size() { return data.size(); }

    bool classificationBit(int row_index) { return data[row_index].second; }
    int countOnes();

    bool isPure();
    void filter(Predicate phi, bool mode);
    double summary();
};

class Predicate {
private:

public:
    bool evaluate(Input x);
};

class PredicateSet {
private:

public:
    Predicate bestSplit(DataSet &training_set);
};


#endif
