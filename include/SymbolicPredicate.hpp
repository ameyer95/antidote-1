#ifndef SYMBOLICPREDICATE_HPP
#define SYMBOLICPREDICATE_HPP

/**
 * This file defines a three-value logic symbolic predicate
 * for numerical features---where a single predicate's threshold
 * could be from a range of values.
 * It largely mirrors Predicate.hpp
 *
 * We use optional<bool> for three-valued logic.
 */

#include "Feature.hpp"
#include <functional> // for std::hash
#include <optional>
#include <iostream>


class SymbolicPredicate {
private:
    unsigned int feature_index;
    FeatureType feature_type;
    // For when feature_type == FeatureType::NUMERIC.
    // Checks lb <= x < ub
    float threshold_lb, threshold_ub;

public:
    SymbolicPredicate(int feature_index); // Sets feature_type = FeatureType::BOOLEAN
    SymbolicPredicate(int feature_index, float threshold_lb, float threshold_ub); // Sets feature_Type = FeatureType::NUMERIC

    // Takes the feature vector, and two bools (feature poisoning for satisfies, feature poisoning for doesn't satisfy)
    // Returns true if unambigously satisfies, false if doesn't satisfy, and {} if satisfy depends on feature poisoning
    std::optional<bool> evaluate(const FeatureVector &x, bool feature_poisoning, float feature_flip_amt) const; // Does not check bounds

    // Our abstract transformers would like to be able to hash these objects, etc
    bool operator ==(const SymbolicPredicate &right) const;
    size_t hash() const;

    unsigned int get_feature_index() const;
    float get_lb() const;
    float get_ub() const;
};

// And a wrapper for SymbolicPredicate::hash so we can conveniently use std::unordered_map
// XXX hashing these may no longer be used
struct hash_SymbolicPredicate {
    size_t operator()(const SymbolicPredicate &p) const {
        return p.hash();
    }
};


/**
 * Member functions below
 */

inline SymbolicPredicate::SymbolicPredicate(int feature_index) {
    this->feature_index = feature_index;
    feature_type = FeatureType::BOOLEAN;
}

inline SymbolicPredicate::SymbolicPredicate(int feature_index, float threshold_lb, float threshold_ub) {
    this->feature_index = feature_index;
    this->threshold_lb = threshold_lb;
    this->threshold_ub = threshold_ub;
    feature_type = FeatureType::NUMERIC;
}

inline std::optional<bool> SymbolicPredicate::evaluate(const FeatureVector &x, bool feature_poisoning, float feature_flip_amt = 0) const {
    // feature_poisoning is true if the feature poisoning index is the same value that phi considers
    switch(feature_type) {
        case FeatureType::BOOLEAN:
            return x[feature_index].getBooleanValue();
        case FeatureType::NUMERIC:
            // given range [lb, ub] the predicate is x<=B for some B in [lb,ub] (we just don't know exactly what B is)
            // return true if x<=B, return false if x>B and return {} if we don't know
            if (feature_poisoning) {
                // In this case, we definitely include x in the filtering
                if (x[feature_index].getNumericValue() <= (threshold_lb - feature_flip_amt)) {
                    return true;
                } else if (x[feature_index].getNumericValue() < threshold_ub + feature_flip_amt) {
                    // Now, x is near the border: could go either way so we have to include it but also will need to increment num_dropout
                    return {};
                } else {
                    return false;
                }
            } else {
                if(x[feature_index].getNumericValue() <= threshold_lb) {
                    return true;
                } else if(x[feature_index].getNumericValue() < threshold_ub) {
                    // In theory this shouldn't happen: the ub is the lb of the next phi, defined to be the next value > lb
                    return {}; // x in [lb,ub] means we don't know whether it'll be included
                } else {
                    return false;
                }
            }
            
        default:
            // XXX this shouldn't happen---it's here to suppress a warning.
            // If adding new FeatureTypes, make sure to add remaining cases.
            return false;
    }
}

inline unsigned int SymbolicPredicate::get_feature_index() const {
    return this->feature_index;
}

inline float SymbolicPredicate::get_lb() const {
    return this->threshold_lb;
}

inline float SymbolicPredicate::get_ub() const {
    return this-> threshold_ub;
}


inline bool SymbolicPredicate::operator ==(const SymbolicPredicate &right) const {
    if(this->feature_index != right.feature_index) {
        return false;
    }
    if(this->feature_type != right.feature_type) {
        return false;
    }
    if(this->feature_type == FeatureType::NUMERIC) {
        if(this->threshold_lb != right.threshold_lb ||
                this->threshold_ub != right.threshold_ub) {
            return false;
        }
    }
    return true;
}

inline size_t SymbolicPredicate::hash() const {
    if(feature_type == FeatureType::NUMERIC) {
        return std::hash<unsigned int>{}(feature_index)
            ^ std::hash<float>{}(threshold_lb)
            ^ std::hash<float>{}(threshold_ub);
    } else { // XXX assuming FeatureType::BOOLEAN
        return std::hash<unsigned int>{}(feature_index);
    }
}

#endif
