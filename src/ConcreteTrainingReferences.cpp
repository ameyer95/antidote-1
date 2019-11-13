#include "ConcreteTrainingReferences.h"
#include "CategoricalDistribution.hpp"
#include "DataReferences.h"
#include "Feature.hpp"
#include "information_math.h"
#include "Predicate.hpp"
#include <algorithm> // for std::all_of, std::sort, ...
#include <list>
#include <optional>
#include <set>
#include <utility>
#include <vector>
using namespace std;


/**
 * Auxiliary methods
 */

inline bool emptyCount(const vector<int> &counts) {
    return all_of(counts.cbegin(), counts.cend(), [](int i){return i == 0;});
}


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

void ConcreteTrainingReferences::computePredicatesAndScores(list<pair<Predicate, double>> &store, int feature_index) const {
    switch(training_references.getFeatureTypes()[feature_index]) {
        // XXX need to make changes here if adding new feature types
        case FeatureType::BOOLEAN:
            computeBooleanFeaturePredicateAndScore(store, feature_index);
            break;
        case FeatureType::NUMERIC:
            computeNumericFeaturePredicatesAndScores(store, feature_index);
            break;
    }
}

void ConcreteTrainingReferences::computeBooleanFeaturePredicateAndScore(list<pair<Predicate, double>> &store, int feature_index) const {
    Predicate phi(feature_index);
    auto counts = splitCounts(phi);
    if(!emptyCount(counts.first) && !emptyCount(counts.second)) {
        store.push_back(make_pair(phi, jointImpurity(counts.first, counts.second)));
    }
}

void ConcreteTrainingReferences::computeNumericFeaturePredicatesAndScores(list<pair<Predicate, double>> &store, int feature_index) const {
    vector<pair<float,int>> value_class_pairs(training_references.size());
    for(unsigned int j = 0; j < training_references.size(); j++) {
        value_class_pairs[j].first = training_references[j].x[feature_index].getNumericValue();
        value_class_pairs[j].second = training_references[j].y;
    }
    if(value_class_pairs.size() < 2) {
        // We do this check so we can safely do a pairwise iteration later
        return;
    }
    std::sort(value_class_pairs.begin(), value_class_pairs.end(),
              [](const pair<float,int> &p1, const pair<float,int> &p2)
              { return p1.first < p2.first; } );
    // value_class_pairs is now in increasing order (with possible multiplicity)
    pair<vector<int>, vector<int>> split_counts = make_pair(vector<int>(training_references.getNumCategories(), 0), sampleCounts());
    for(auto i = value_class_pairs.begin(); i + 1 != value_class_pairs.end(); i++) {
        // We will consider a threshold between i and i+1.
        // Here, i is crossing to the smaller side of the treshold (split_counts.first)
        // and we make an update to the count of classes accordingly.
        split_counts.first[i->second]++;
        split_counts.second[i->second]--;
        // Next, if i and i+1 have distinct float values, we'll consider a predicate here
        if(i->first == (i+1)->first) {
            continue;
        }
        float threshold = (i->first + (i+1)->first) / 2;
        Predicate phi(feature_index, threshold);
        store.push_back(make_pair(phi, jointImpurity(split_counts.first, split_counts.second)));
    }
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

CategoricalDistribution<double> ConcreteTrainingReferences::summary() const {
    vector<int> counts = sampleCounts();
    return estimateCategorical(counts);
}

optional<Predicate> ConcreteTrainingReferences::bestSplit() const {
    double best_score;
    optional<Predicate> best_predicate = {};
    // For each feature index, we'll have a list of the scores for each gathered predicate
    vector<list<pair<Predicate, double>>> feature_scores(training_references.getFeatureTypes().size());
    for(int i = 0; i < training_references.getFeatureTypes().size(); i++) {
        computePredicatesAndScores(feature_scores[i], i); // Only populates with non-trivial splits
    }

    for(auto i = feature_scores.cbegin(); i != feature_scores.cend(); i++) {
        for(auto j = i->cbegin(); j != i->cend(); j++) {
            if(!best_predicate.has_value() || j->second < best_score) { // Minimize joint Impurity
                best_score = j->second;
                best_predicate = j->first;
            }
        }
    }
    return best_predicate;
}
