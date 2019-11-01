#include "CategoricalDistribution.h"
#include <numeric>
#include <string>
#include <vector>

std::string CategoricalDistribution::toString() const {
    std::string ret = "{";
    for(unsigned int i = 0; i < p.size(); i++) {
        ret += std::to_string(i) + ":" + std::to_string(p[i]);
        if(i + 1 < p.size()) {
            ret += ", ";
        }
    }
    ret += "}";
    return ret;
}

CategoricalDistribution CategoricalDistribution::estimateFrom(std::vector<int> counts) {
    CategoricalDistribution ret(counts.size());
    int total = accumulate(counts.begin(), counts.end(), 0);
    for(unsigned int i = 0; i < counts.size(); i++) {
        ret[i] = (double)counts[i] / total;
    }
    return ret;
}

