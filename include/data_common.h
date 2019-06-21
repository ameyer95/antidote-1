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
    DataReferences(const vector<T> *data); // Does not handle deallocation

    const T& operator [](unsigned int i) const { return (*data)[indices[i]]; }
    void remove(int index) { indices.erase(indices.begin() + index); }
    const unsigned int size() { return indices.size(); }
};


template <typename T>
DataReferences<T>::DataReferences(const vector<T> *data) {
    this->data = data;
    indices.reserve(data->size());
    for(int i = 0; i < data->size(); i++) {
        indices.push_back(i);
    }
}

#endif
