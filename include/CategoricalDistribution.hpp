#ifndef CATEGORICALDISTRIBUTION_HPP
#define CATEGORICALDISTRIBUTION_HPP

/**
 * In general, we imagine that a training set T has rows consisting of elements
 * in some space X x Y.
 * Here, X is a feature vector, and Y is the class label.
 * We assume that Y is a finite, discrete space with no interesting structure:
 * accordingly, we identify Y with integers {1, ..., k}.
 * (Binary classification, typically {0,1}, will be represented with k = 2.)
 * In this file, we provide the generalized data type
 * for categorical distributions (over Y).
 */

#include <vector>

// We don't need a specific class for Y itself,
// since, again, we just use ints {1, ..., k}.


/**
 * For a k-categorical output, stores k floats.
 * It does no checking to ensure this distribution is well-defined.
 * This class just incompletely wraps a std::vector,
 * hiding ways to change size,
 * because there's otherwise no appropriately polymorphic tuple.
 */
class CategoricalDistribution {
private:
    std::vector<float> p; // Will be a fixed size, as determined by constructor.

public:
    // Initializes to size-many 0's:
    CategoricalDistribution(unsigned int size) { p = std::vector<float>(size, 0); }

    unsigned int size() const { return p.size(); }
    float& operator [](unsigned int i) { return p[i]; } // Note: no bounds check.
};


#endif
