#ifndef RAWDATA_H
#define RAWDATA_H

#include <utility>
using namespace std;

template <typename X, typename Y>
class RawData {
public:
    // Read-only [] access
    virtual const pair<const X, const Y> operator [](unsigned int i) const = 0;
};

#endif
