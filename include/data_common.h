#ifndef DATA_COMMON_H
#define DATA_COMMON_H

#include <vector>
using namespace std;


// T is the type of each data element
template <typename T>
class DataReferences {
private:
    const vector<T> *data;
    vector<int> indices;

public:
    DataReferences() { data = NULL; indices = {}; }
    DataReferences(const vector<T> *data); // Does not handle deallocation
    DataReferences(const vector<T> *data, const vector<int> &indices) { this->data = data; this->indices = indices; }

    const T& operator [](unsigned int i) const { return (*data)[indices[i]]; }
    void remove(int index) { indices.erase(indices.begin() + index); }
    unsigned int size() const { return indices.size(); }

    static DataReferences<T> set_union(const DataReferences<T> &e1, const DataReferences<T> &e2);
};


template <typename T>
DataReferences<T>::DataReferences(const vector<T> *data) {
    this->data = data;
    indices.reserve(data->size());
    for(int i = 0; i < data->size(); i++) {
        indices.push_back(i);
    }
}

template <typename T>
DataReferences<T> DataReferences<T>::set_union(const DataReferences<T> &e1, const DataReferences<T> &e2) {
    // XXX strong assumption that e1.data == e2.data
    vector<int>::const_iterator i1, i2;
    vector<int> ret_indices;
    i1 = e1.indices.begin();
    i2 = e2.indices.begin();
    for(int i = 0; i < e1.data->size(); i++) {
        if((i1 != e1.indices.end() && *i1 == i) || (i2 != e2.indices.end() && *i2 == i)) {
            ret_indices.push_back(i);
        }
        while(i1 != e1.indices.end() && *i1 <= i) {
            i1++;
        }
        while(i2 != e2.indices.end() && *i2 <= i) {
            i2++;
        }
    }
    return DataReferences<T>(e1.data, ret_indices);
}

#endif
