#include "DataReferences.h"
#include <algorithm> // for std::min_element
#include <vector>

DataReferences::DataReferences(const DataSet *data_set) {
    this->data_set = data_set;
    indices.reserve(data_set->rows.size());
    for(unsigned int i = 0; i < data_set->rows.size(); i++) {
        indices.push_back(i);
    }
}

DataReferences::DataReferences(const DataSet *data_set, const std::vector<int> &indices) {
    this->data_set = data_set;
    this->indices = indices;
}

DataReferences DataReferences::set_union(const DataReferences &e1, const DataReferences &e2) {
    // XXX strong assumption that e1.data_set == e2.data_set
    // and the invariant that DataReferences::indices are sorted
    std::vector<int>::const_iterator i1, i2;
    std::vector<int> ret_indices;

    i1 = e1.indices.begin();
    i2 = e2.indices.begin();
    while(i1 != e1.indices.end() || i2 != e2.indices.end()) {
        std::vector<int> candidates;
        if(i1 != e1.indices.end()) {
            candidates.push_back(*i1);
        }
        if(i2 != e2.indices.end()) {
            candidates.push_back(*i2);
        }
        int current = *std::min_element(candidates.begin(), candidates.end());
        ret_indices.push_back(current);
        if(i1 != e1.indices.end() && *i1 == current) {
            i1++;
        }
        if(i2 != e2.indices.end() && *i2 == current) {
            i2++;
        }
    }
    return DataReferences(e1.data_set, ret_indices);
}
