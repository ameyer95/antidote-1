#ifndef CATEGORICALDISTRIBUTION_H
#define CATEGORICALDISTRIBUTION_H

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

#include <string>
#include <vector>

// We don't need a specific class for Y itself,
// since, again, we just use k ints {0, ..., k-1}.


/**
 * For a k-categorical output, stores k doubles.
 * It does no checking to ensure this distribution is well-defined.
 * This class just incompletely wraps a std::vector,
 * hiding ways to change size,
 * because there's otherwise no appropriately polymorphic tuple.
 */
class CategoricalDistribution {
private:
    std::vector<double> p; // Will be a fixed size, as determined by constructor.

public:
    // Initializes to size-many 0's:
    CategoricalDistribution(unsigned int size) { p = std::vector<double>(size, 0); }
    CategoricalDistribution() { p = std::vector<double>({0}); }

    unsigned int size() const { return p.size(); }
    double& operator [](unsigned int i) { return p[i]; } // Note: no bounds check.
    const double& operator [](unsigned int i) const { return p[i]; }

    std::string toString() const;

    static CategoricalDistribution estimateFrom(std::vector<int> counts);
};


#endif
