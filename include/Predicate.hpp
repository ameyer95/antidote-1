#ifndef PREDICATE_HPP
#define PREDICATE_HPP

/*
 * In general, we imagine that a training set T has rows consisting of elements
 * in some space X x Y.
 * Here, X is a feature vector, and Y is the class label.
 * During training, there are predicates that operate over X:
 * this file provides the general data type for such predicates.
 */

#include "Feature.hpp"


class Predicate {
private:
    unsigned int feature_index;
    FeatureType feature_type;
    float threshold; // For when feature_type == FeatureType::NUMERIC

public:
    Predicate(int feature_index); // Sets feature_type = FeatureType::BOOLEAN
    Predicate(int feature_index, float threshold); // Sets feature_Type = FeatureType::NUMERIC

    bool evaluate(const FeatureVector &x) const; // Does not check bounds
};


/**
 * Member functions below
 */

inline Predicate::Predicate(int feature_index) {
    this->feature_index = feature_index;
    feature_type = FeatureType::BOOLEAN;
}

inline Predicate::Predicate(int feature_index, float threshold) {
    this->feature_index = feature_index;
    this->threshold = threshold;
    feature_type = FeatureType::NUMERIC;
}

inline bool Predicate::evaluate(const FeatureVector &x) const {
    // XXX does no check to ensure this->feature_type is consistent with x[feature_index]'s
    switch(feature_type) {
        case FeatureType::BOOLEAN:
            return x[feature_index].getBooleanValue();
        case FeatureType::NUMERIC:
            return x[feature_index].getNumericValue() <= threshold;
        default:
            // XXX this shouldn't happen---it's here to suppress a warning.
            // If adding new FeatureTypes, make sure to add remaining cases.
            return false;
    }
}

#endif
