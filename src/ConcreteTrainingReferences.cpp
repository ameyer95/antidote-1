#include "ConcreteTrainingReferences.h"
#include "CategoricalDistribution.h"
#include "DataReferences.h"
#include "Feature.hpp"
#include "information_math.h"
#include "Predicate.hpp"
#include <algorithm> // for std::all_of
#include <list>
#include <optional>
#include <set>
#include <utility>
#include <vector>
using namespace std;

/**
 * Forward declarations for auxiliary methods
 */

bool emptyCount(const vector<int> &counts);
void storePredicatesForFeatureValues(list<Predicate> &ret, int index, vector<float> &feature_values);


/**
 * Private member functions
 */

vector<int> ConcreteTrainingReferences::sampleCounts() const {
    vector<int> ret(training_references.getNumCategories(), 0);
    for(unsigned int i = 0; i < training_references.size(); i++) {
        ret[training_references[i].y] += 1;
    }
    return ret;
}

pair<vector<int>, vector<int>> ConcreteTrainingReferences::splitCounts(const Predicate &phi) const {
    pair<vector<int>, vector<int>> ret(vector<int>(training_references.getNumCategories(), 0),
                                       vector<int>(training_references.getNumCategories(), 0));
    vector<int> *samples_ptr;
    for(unsigned int i = 0; i < training_references.size(); i++) {
        // Convention here is that satisfying the predicate corresponds to the second pair element
        samples_ptr = phi.evaluate(training_references[i].x) ? &(ret.second) : &(ret.first);
        (*samples_ptr)[training_references[i].y] += 1;
    }
    return ret;
}

list<Predicate> ConcreteTrainingReferences::gatherPredicates() const {
    list<Predicate> ret;
    FeatureVectorHeader header = training_references.getFeatureTypes();
    for(unsigned int i = 0; i < header.size(); i++) {
        switch(header[i]) {
            // XXX need to make changes here if adding new feature types
            case FeatureType::BOOLEAN:
                ret.push_back(Predicate(i));
                break;
            case FeatureType::NUMERIC:
                vector<float> feature_values(training_references.size());
                for(unsigned int j = 0; j < training_references.size(); j++) {
                    feature_values[j] = training_references[j].x[i].getNumericValue();
                }
                storePredicatesForFeatureValues(ret, i, feature_values);
                break;
        }
    }
    return ret;
}


/**
 * Public member functions
 */

bool ConcreteTrainingReferences::isPure() const {
    // XXX What if training set is empty?
    int category = training_references[0].y;
    for(unsigned int i = 1; i < training_references.size(); i++) {
        if(category != training_references[i].y) {
            return false;
        }
    }
    return true;
};

void ConcreteTrainingReferences::filter(const Predicate &phi, bool mode) {
    bool remove, result;
    // TODO this iterative removal is potentially inefficient; consider a linked list?
    for(unsigned int i = 0; i < training_references.size(); i++) {
        result = phi.evaluate(training_references[i].x);
        remove = (mode != result);
        if(remove) {
            training_references.remove(i);
            i--;
        }
    }
}

CategoricalDistribution ConcreteTrainingReferences::summary() const {
    vector<int> counts = sampleCounts();
    return CategoricalDistribution::estimateFrom(counts);
}

optional<Predicate> ConcreteTrainingReferences::bestSplit() const {
    double best_score, current_score;
    optional<Predicate> best_predicate = {};
    list<Predicate> predicates = gatherPredicates();
    for(auto i = predicates.cbegin(); i != predicates.cend(); i++) {
        auto counts = splitCounts(*i);
        if(!emptyCount(counts.first) && !emptyCount(counts.second)) {
            current_score = jointImpurity(counts.first, counts.second);
            if(!best_predicate.has_value() || current_score < best_score) { // Minimize joint Impurity
                best_score = current_score;
                best_predicate = *i;
            }
        }
    }
    return best_predicate;
}


/**
 * Auxiliary methods
 */

inline bool emptyCount(const vector<int> &counts) {
    return all_of(counts.cbegin(), counts.cend(), [](int i){return i == 0;});
}

void storePredicatesForFeatureValues(list<Predicate> &ret, int index, vector<float> &feature_values) {
    if(feature_values.size() == 0) {
        return;
    }

    // std::set sorts, so we can use it to deduplicate + sort
    set<float> unique_values(feature_values.begin(), feature_values.end());
    feature_values.assign(unique_values.begin(), unique_values.end());

    // For each adjacent pair, store a predicate using their average
    for(auto i = feature_values.cbegin(); i + 1 != feature_values.cend(); i++) {
        float threshold = (*i + *(i+1)) / 2;
        ret.push_back(Predicate(index, threshold));
    }
}
