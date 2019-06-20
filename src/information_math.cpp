#include "information_math.h"
#include <utility>
using namespace std;

double impurity(const pair<int, int> &counts) {
    // Assume pair::first is 0 and pair::second is 1, but equivalent either way
    double p = (double)counts.second / (counts.first + counts.second);
    return p * (1-p) / 2; // Gini impurity
}

double informationGain(const pair<int, int> &counts1, const pair<int, int> &counts2) {
    pair<int, int> joint_counts(counts1.first + counts2.first, counts1.second + counts2.second);
    double ret = impurity(joint_counts) -
                 ((counts1.first + counts1.second) * impurity(counts1) +
                  (counts2.first + counts2.second) * impurity(counts2))
                 / (joint_counts.first + joint_counts.second);
    return ret;
}
