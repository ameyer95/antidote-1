#include "DataSet.h"


// Everything is currently hard-coded for binary classification and boolean features.

inline int DataSet::rowOffset(int row_index) {
    return row_size * row_index;
}

DataSet::DataSet(bool *bits, int num_bits, int row_size) {
    data.bits = bits;
    data.num_bits = num_bits;
    this->row_size = row_size;
    num_rows = num_bits / row_size;
}

inline bool DataSet::classificationBit(int row_index) {
    return data.bits[rowOffset(row_index) + row_size - 1];
}

int DataSet::countOnes() {
    int count = 0;
    for(int i = 0; i < num_rows; i++) {
        if(classificationBit(i)) {
            count++;
        }
    }
    return count;
}

bool DataSet::isPure() {
    int count = countOnes();
    return count == 0 || count == num_rows;
}

void DataSet::filter(Predicate phi, bool mode) {
    //TODO
}

double DataSet::summary() {
    //TODO
}

bool Predicate::evaluate(Input x) {
    //TODO
}

Predicate PredicateSet::bestSplit(DataSet &training_set) {
    //TODO
}
