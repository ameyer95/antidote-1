#ifndef FEATURE_HPP
#define FEATURE_HPP

/**
 * In general, we imagine that a training set T has rows consisting of elements
 * in some space X x Y.
 * Here, X is a feature vector, and Y is the class label.
 * In this file, we provide the generalized data type
 * for individual features in the feature vector.
 *
 * Finally, we provide some framework for feature vectors:
 * members of X should be FeatureVector instances.
 *
 * Adding additional features requires:
 *   1) A new enum category in FeatureType
 *   2) A new union element in FeatureUnion
 *   3) A new assignment overload in Feature
 *   4) A new accessor in Feature
 *   5) Ensuring Predicate.hpp supports it
 */

#include <vector>


// Each feature type needs an entry in the following enum:
enum class FeatureType {
    BOOLEAN,
    NUMERIC, // Any continuous-valued feature; internally, we'll use floats.
};


// Next, the actual union of the individual feature types.
union FeatureUnion {
    bool boolean_value;
    float numeric_value;
};


// All our values will use FeatureType to track
// which element of FeatureUnion is live.
class Feature {
private:
    FeatureType type;
    FeatureUnion value;

public:
    // For each value possibility in FeatureUnion,
    // we provide an assignment overload.
    Feature& operator=(const bool& other);
    Feature& operator=(const float& other);

    const FeatureType& getType() const { return type; }

    // For each value possibility in FeatureUnion,
    // we provide an accessor.
    // XXX No check for correct FeatureType
    const bool& getBooleanValue() const { return value.boolean_value; }
    const float& getNumericValue() const { return value.numeric_value; }
};


// The intention is for a FeatureVectorHeader to specify the FeatureTypes
// used in parallel FeatureVector instances.
typedef std::vector<FeatureType> FeatureVectorHeader;
typedef std::vector<Feature> FeatureVector;


/**
 * Member definitions for Feature
 */

inline Feature& Feature::operator=(const bool& other) {
    type = FeatureType::BOOLEAN;
    value.boolean_value = other;
    return *this;
}

inline Feature& Feature::operator=(const float& other) {
    type = FeatureType::NUMERIC;
    value.numeric_value = other;
    return *this;
}

#endif
