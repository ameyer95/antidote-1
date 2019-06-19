#ifndef DATACOMMON_H
#define DATACOMMON_H

#include <vector>
using namespace std;


// T is the type of each data element
template <typename T>
class DataReferences {
private:
    vector<T> *data;
    vector<int> indices;
public:
    const T& operator [](unsigned int i) const { return (*data)[indices[i]]; }
    void remove(int index) { indices.erase(indices.begin() + index); }
    const unsigned int size() { return indices.size(); }
};


#endif
