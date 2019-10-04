#ifndef RAWDATA_H
#define RAWDATA_H

#include <utility>


template <typename X, typename Y>
class RawData {
public:
    // Read-only [] access
    virtual const std::pair<const X, const Y> operator [](unsigned int i) const = 0;
    virtual unsigned int size() const = 0;
};


#endif
