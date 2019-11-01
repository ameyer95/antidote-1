#ifndef CATEGORICALDISTRIBUTION_HPP
#define CATEGORICALDISTRIBUTION_HPP

/**
 * In general, we imagine that a training set T has rows consisting of elements
 * in some space X x Y.
 * Here, X is a feature vector, and Y is the class label.
 * We assume that Y is a finite, discrete space with no interesting structure:
 * accordingly, we identify Y with integers {0, ..., k-1}.
 * (Binary classification, typically {0,1}, will be represented with k = 2.)
 * In this file, we provide the generalized data type
 * for categorical distributions (over Y).
 */

#include <vector>

// We don't need a specific class for Y itself,
// since, again, we just use k ints {0, ..., k-1}.


/**
 * For a k-categorical output, stores k doubles (or Interval<double>, etc).
 * It does no checking to ensure this distribution is well-defined.
 * This class just incompletely wraps a std::vector,
 * hiding ways to change size,
 * because there's otherwise no appropriately polymorphic tuple.
 */
template<typename T>
class CategoricalDistribution {
private:
    std::vector<T> p; // Will be a fixed size, as determined by constructor.

public:
    // Initializes to size-many uninitialized elements:
    CategoricalDistribution(unsigned int size) { p = std::vector<T>(size); }
    CategoricalDistribution() {}

    unsigned int size() const { return p.size(); }
    T& operator [](unsigned int i) { return p[i]; } // Note: no bounds check.
    const T& operator [](unsigned int i) const { return p[i]; }
};


#endif
