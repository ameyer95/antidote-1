#ifndef DATAREFERENCES_H
#define DATAREFERENCES_H

#include <algorithm> // for min_element
#include <vector>


// T is the type of each data element
template <typename T>
class DataReferences {
private:
    const std::vector<T> *data;
    std::vector<int> indices;

public:
    DataReferences() { data = NULL; indices = {}; }
    DataReferences(const std::vector<T> *data); // Does not handle deallocation
    DataReferences(const std::vector<T> *data, const std::vector<int> &indices) { this->data = data; this->indices = indices; }

    const T& operator [](unsigned int i) const { return (*data)[indices[i]]; }
    void remove(int index) { indices.erase(indices.begin() + index); }
    unsigned int size() const { return indices.size(); }

    static DataReferences<T> set_union(const DataReferences<T> &e1, const DataReferences<T> &e2);
};


template <typename T>
DataReferences<T>::DataReferences(const std::vector<T> *data) {
    this->data = data;
    indices.reserve(data->size());
    for(unsigned int i = 0; i < data->size(); i++) {
        indices.push_back(i);
    }
}

template <typename T>
DataReferences<T> DataReferences<T>::set_union(const DataReferences<T> &e1, const DataReferences<T> &e2) {
    // XXX strong assumption that e1.data == e2.data
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
    return DataReferences<T>(e1.data, ret_indices);
}

#endif
