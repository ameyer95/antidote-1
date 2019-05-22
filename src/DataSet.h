#ifndef DATASET_H
#define DATASET_H


// Everything is currently hard-coded for binary classification and boolean features.

class Predicate;

struct BitVector {
    bool *bits;
    int num_bits;
};

typedef BitVector Input;


class DataSet {
private:
    BitVector data;
    int num_rows, row_size; // How many rows and how many bools per row

    int rowOffset(int row_index);
public:
    DataSet(bool *bits, int num_bits, int row_size);
    // TODO who should handle deallocation?

    bool classificationBit(int row_index);
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
