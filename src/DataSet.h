#ifndef DATASET_H
#define DATASET_H


// Everything is currently hard-coded for binary classification and boolean features.

class Input;
class DataRow;
class DataSet;
class Predicate;
class PredicateSet;


class Input {
private:

public:
};

class DataRow {
private:

public:
};

class DataSet {
private:

public:
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
