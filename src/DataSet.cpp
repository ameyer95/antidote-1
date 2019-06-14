#include "DataSet.h"
#include <vector>
#include <utility>
#include <math.h> // For isnan
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
        data.push_back(buildRow(bits + i*row_size, row_size));
    }
}

DataSet::DataSet(vector<DataRow> &data) {
    this->data = data; // XXX large copy
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

void DataSet::filter(Predicate &phi, bool mode) {
    bool remove, result;
    // XXX this iterative removal is potentially very inefficient; consider a linked list
    for(vector<DataRow>::iterator i = data.begin(); i != data.end(); i++) {
        result = phi.evaluate(i->first);
        remove = mode ? result : !result;
        if(remove) {
            data.erase(i--); // Decrement iterator after passing, but before execution
        }
    }
}

double DataSet::summary() {
    return (double)countOnes() / data.size();
}

double DataSet::impurity() {
    double p = summary();
    return p * (1-p) / 2; // Gini impurity
}

pair<DataSet*, DataSet*> DataSet::split(Predicate &phi) {
    // XXX both this and the constructor copy a vector
    vector<DataRow> pos, neg;
    for(vector<DataRow>::iterator i = data.begin(); i != data.end(); i++) {
        if(phi.evaluate(i->first)) {
            pos.push_back(*i);
        } else {
            neg.push_back(*i);
        }
    }
    return make_pair(new DataSet(pos), new DataSet(neg));
}

bool Predicate::evaluate(Input x) {
    return x[bit_index];
}

Predicate* PredicateSet::bestSplit(DataSet &training_set) {
    double best_score, current_score;
    Predicate *best_predicate = NULL;
    for(vector<Predicate>::iterator i = predicates->begin(); i != predicates->end(); i++) {
        current_score = this->informationGain(*i, training_set);
        //XXX the semantics of this conditional,
        // and what to do when all splits are trivial or provide no gain,
        // are corner cases that really need more careful consideration
        if(best_predicate == NULL || isnan(best_score) || best_score < current_score) {
            best_score = current_score;
            best_predicate = &(*i); // Need the address of the iterator's current element
        }
    }
    return best_predicate;
}

double PredicateSet::informationGain(Predicate &phi, DataSet &training_set) {
    pair<DataSet*, DataSet*> split = training_set.split(phi);
    return training_set.impurity() - (
            split.first->size() * split.first->impurity() +
            split.second->size() * split.second->impurity()
            ) / training_set.size();
}
