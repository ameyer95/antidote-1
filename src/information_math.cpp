#include "information_math.h"
#include "Interval.h"
#include <algorithm>
#include <utility>
using namespace std;

double impurity(const pair<int, int> &counts) {
    // Assume pair::first is 0 and pair::second is 1, but equivalent either way
    double p = (double)counts.second / (counts.first + counts.second);
    return p * (1-p) / 2; // Gini impurity
}

Interval<double> impurity(const pair<int, int> &counts, int num_dropout) {
    Interval<double> p;
    if(counts.first + counts.second == num_dropout) {
        p = Interval<double>(0, 1);
    } else {
        Interval<double> num(max(counts.first - num_dropout, 0), counts.first);
        Interval<double> den(counts.first + counts.second - num_dropout, counts.first + counts.second);
        p = num / den;
    }
    return p * (Interval<double>(1) - p) * Interval<double>(2);
}

double jointImpurity(const pair<int, int> &counts1, const pair<int, int> &counts2) {
    return (counts1.first + counts1.second) * impurity(counts1)
        + (counts2.first + counts2.second) * impurity(counts2);
}

Interval<double> jointImpurity(const pair<int, int> &counts1, int num_dropout1, const pair<int, int> &counts2, int num_dropout2) {
    Interval<double> size1(counts1.first + counts1.second - num_dropout1, counts1.first + counts1.second);
    Interval<double> size2(counts2.first + counts2.second - num_dropout2, counts2.first + counts2.second);
    return size1 * impurity(counts1, num_dropout1) + size2 * impurity(counts2, num_dropout2);
}

double informationGain(const pair<int, int> &counts1, const pair<int, int> &counts2) {
    pair<int, int> joint_counts(counts1.first + counts2.first, counts1.second + counts2.second);
    double ret = impurity(joint_counts) -
                 ((counts1.first + counts1.second) * impurity(counts1) +
                  (counts2.first + counts2.second) * impurity(counts2))
                 / (joint_counts.first + joint_counts.second);
    return ret;
}
