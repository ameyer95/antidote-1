#ifndef DATAREFERENCES_H
#define DATAREFERENCES_H

/**
 * While DataSet.hpp contains a struct for representing a data set,
 * we often want manipulate the data set, or have many copies of it, etc.
 * Its much more efficient to introduce a single level of indirection:
 * accordingly, this file defines a data set interface that keeps track of
 * element addresses from a single, read-only DataSet.
 */

#include "DataSet.hpp"


class DataReferences {
private:
    const DataSet *data_set; // Does not handle deallocation
    std::vector<int> indices;

public:
    DataReferences() { data_set = NULL; indices = {}; }
    DataReferences(const DataSet *data_set);
    DataReferences(const DataSet *data_set, const std::vector<int> &indices);

    // Some accessors for the underlying DataSet fields
    const FeatureVectorHeader& getFeatureTypes() const { return data_set->feature_types; }
    int getNumCategories() const { return data_set->num_categories; }

    const DataRow& operator [](unsigned int i) const { return data_set->rows[indices[i]]; }
    void remove(int index) { indices.erase(indices.begin() + index); }
    unsigned int size() const { return indices.size(); }

    static DataReferences set_union(const DataReferences &e1, const DataReferences &e2);
};


#endif
