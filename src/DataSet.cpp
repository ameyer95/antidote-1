#include "DataSet.h"
#include <vector>
#include <utility>
using namespace std;


// Everything is currently hard-coded for binary classification and boolean features.

// Strongly assumes that the class label is a single, last bool
// Also, uses that DataRow is just a pair typedef, etc
DataRow buildRow(bool *bits, int row_size) {
    vector<bool> x;
    x.reserve(row_size - 1);
    for (int i = 0; i < row_size - 1; i++) {
        x.push_back(bits[i]);
    }
    return make_pair(x, bits[row_size - 1]);
}

// bits should be some array of bools -- the flattened two-dimension structure
DataSet::DataSet(bool *bits, int num_bits, int row_size) {
    int num_rows = num_bits / row_size;
    data.reserve(num_rows);
    for(int i = 0; i < num_rows; i++) {
        data.push_back(buildRow(bits + i, row_size));
    }
}

int DataSet::countOnes() {
    int count = 0;
    for(int i = 0; i < data.size(); i++) {
        if(classificationBit(i)) {
            count++;
        }
    }
    return count;
}

bool DataSet::isPure() {
    int count = countOnes();
    return count == 0 || count == data.size();
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
