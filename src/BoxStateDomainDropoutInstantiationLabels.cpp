#include "BoxStateDomainDropoutInstantiationLabels.h"
#include "Feature.hpp"
#include "information_math.h"
#include "Interval.h"
#include <algorithm>
#include <list>
#include <numeric> // for std::accumulate
#include <set>
#include <utility>
#include <vector>

/**
 * TrainingReferencesWithDropout members
 */

TrainingReferencesWithDropoutLabels::TrainingReferencesWithDropoutLabels(DataReferences training_references, int num_dropout) {
    this->training_references = training_references;
    this->num_dropout = num_dropout;
}

std::vector<int> TrainingReferencesWithDropoutLabels::baseCounts() const {
    std::vector<int> counts(training_references.getNumCategories(), 0);
    for(unsigned int i = 0; i < training_references.size(); i++) {
        counts[training_references[i].y]++;
    }
    return counts;
}

std::pair<TrainingReferencesWithDropoutLabels::DropoutCounts, TrainingReferencesWithDropoutLabels::DropoutCounts> TrainingReferencesWithDropoutLabels::splitCounts(const SymbolicPredicate &phi) const {
    std::pair<DropoutCounts, DropoutCounts> ret;
    ret.first.counts = std::vector<int>(training_references.getNumCategories(), 0);
    ret.second.counts = std::vector<int>(training_references.getNumCategories(), 0);
    DropoutCounts *d_ptr;
    for(unsigned int i = 0; i < training_references.size(); i++) {
        std::optional<bool> result = phi.evaluate(training_references[i].x);
        int category = training_references[i].y;
        // XXX it should be an invariant of the way predicates are selected
        // that we always have result.has_value()
        // but this could be done more safely
        d_ptr = result.value() ? &(ret.second) : &(ret.first);
        d_ptr->counts[category]++;
    }
    // Ensure num_dropouts are well-defined XXX this is more complicated in the 3-valued case
    // except that it's not---again, there should be an invariant that we never fall into
    // maybe cases in this portion of the code
    std::vector<DropoutCounts*> iters = {&(ret.first), &(ret.second)};
    for(auto i = iters.begin(); i != iters.end(); i++) {
        (*i)->num_dropout = std::min(num_dropout, std::accumulate((*i)->counts.cbegin(), (*i)->counts.cend(), 0));
    }
    return ret;
}

TrainingReferencesWithDropoutLabels TrainingReferencesWithDropoutLabels::pureSetRestriction(std::list<int> pure_possible_classes) const {
    DataReferences training_copy = training_references;
    int num_removed = 0;
    for(unsigned int i = 0; i < training_copy.size(); i++) {
        const int current_y = training_copy[i].y;
        if(std::none_of(pure_possible_classes.cbegin(), pure_possible_classes.cend(),
                    [&current_y](int y) { return y == current_y; })) {
            training_copy.remove(i);
            num_removed++;
            i--;
        }
    }
    // We will only call this when it's guaranteed to be non-trivial,
    // so we need not check that num_removed <= num_dropout
    return TrainingReferencesWithDropoutLabels(training_copy, num_dropout - num_removed);
}

TrainingReferencesWithDropoutLabels TrainingReferencesWithDropoutLabels::filter(const SymbolicPredicate &phi, bool positive_flag) const {
    TrainingReferencesWithDropoutLabels ret(*this);
    bool remove;
    std::optional<bool> result;
    int num_maybes = 0;
    for(unsigned int i = 0; i < ret.training_references.size(); i++) {
        result = phi.evaluate(ret.training_references[i].x);
        if(!result.has_value()) {
            num_maybes++;
        }
        remove = result.has_value() && (positive_flag != result.value());
        if(remove) {
            ret.training_references.remove(i);
            i--;
        }
    }
    ret.num_dropout = std::min(ret.num_dropout + num_maybes, (int)ret.training_references.size());
    return ret;
}

/**
 * TrainingSetDropoutDomainLabels members
 */

TrainingReferencesWithDropoutLabels TrainingSetDropoutDomainLabels::meetImpurityEqualsZero(const TrainingReferencesWithDropoutLabels &element) const {
    if(isBottomElement(element)) {
        return element;
    }
    std::vector<int> counts = element.baseCounts();
    std::list<int> pure_possible_classes(0);
    for(unsigned int i = 0; i < counts.size(); i++) {
        // It's possible that all but the i-class elements could be removed
        if(element.training_references.size() - counts[i] <= element.num_dropout) {
            pure_possible_classes.push_back(i);
        }
    }

    // If no pure class is possible, we return a bottom element
    if(pure_possible_classes.size() == 0) {
        return TrainingReferencesWithDropoutLabels();
    }
    // If any number of classes are possible, our abstraction is too coarse
    // to do better than returning the restriction to just those classes,
    // so we soundly overapproximate by doing exactly that
    else if(pure_possible_classes.size() == counts.size()) {
        return element;
    } else {
        return element.pureSetRestriction(pure_possible_classes);
    }
}

TrainingReferencesWithDropoutLabels TrainingSetDropoutDomainLabels::meetImpurityNotEqualsZero(const TrainingReferencesWithDropoutLabels &element) const {
    // Our abstraction is not capable of expressing this precisely
    return element;
}

bool TrainingSetDropoutDomainLabels::isBottomElement(const TrainingReferencesWithDropoutLabels &element) const {
    return element.training_references.size() == 0;
}

TrainingReferencesWithDropoutLabels TrainingSetDropoutDomainLabels::binary_join(const TrainingReferencesWithDropoutLabels &e1, const TrainingReferencesWithDropoutLabels &e2) const {
    if(isBottomElement(e1)) {
        return e2;
    } else if(isBottomElement(e2)) {
        return e1;
    }
    DataReferences d = DataReferences::set_union(e1.training_references, e2.training_references);
    int n1 = d.size() - e2.training_references.size() + e2.num_dropout; // Note |(T1 U T2) \ T1| = |T2 \ T1|
    int n2 = d.size() - e1.training_references.size() + e1.num_dropout;
    return TrainingReferencesWithDropoutLabels(d, std::max(n1, n2));
}

/**
 * PredicateSetDomain members
 */

PredicateAbstractionLabels PredicateSetDomainLabels::meetPhiIsBottom(const PredicateAbstractionLabels &element) const {
    bool contains_bottom = std::any_of(element.cbegin(), element.cend(),
            [](std::optional<SymbolicPredicate> phi){ return !phi.has_value(); });
    if(contains_bottom) {
        std::optional<SymbolicPredicate> bottom = {};
        return PredicateAbstractionLabels({bottom}); // The vector of a single bottom element.
    } else {
        return PredicateAbstractionLabels(); // The empty vector
    }
}

PredicateAbstractionLabels PredicateSetDomainLabels::meetPhiIsNotBottom(const PredicateAbstractionLabels &element) const {
    PredicateAbstractionLabels phis(element); // Make a copy
    // Iterate over the copy and delete any bottom element
    for(auto i = phis.begin(); i != phis.end(); i++) {
        if(!i->has_value()) { // It should be an invariant of the code that this only happens once, but...
            phis.erase(i--);
        }
    }
    return phis;
}

PredicateAbstractionLabels PredicateSetDomainLabels::meetXModelsPhi(const PredicateAbstractionLabels &element, const FeatureVector &x) const {
    if(isBottomElement(element)) {
        return element;
    }
    PredicateAbstractionLabels phis;
    for(auto i = element.cbegin(); i != element.cend(); i++) {
        // The grammar should enforce that we always have i->has_value()
        std::optional<bool> result = i->value().evaluate(x);
        // With three-valued logic, we inlude "maybe" (!result.has_value())
        if(!result.has_value() || result.value()) {
            phis.push_back(*i);
        }
    }
    return phis;
}

PredicateAbstractionLabels PredicateSetDomainLabels::meetXNotModelsPhi(const PredicateAbstractionLabels &element, const FeatureVector &x) const {
    if(isBottomElement(element)) {
        return element;
    }
    PredicateAbstractionLabels phis;
    for(auto i = element.cbegin(); i != element.cend(); i++) {
        // The grammar should enforce that we always have i->has_value()
        std::optional<bool> result = i->value().evaluate(x);
        // With three-valued logic, we inlude "maybe" (!result.has_value())
        if(!result.has_value() || !result.value()) { // This is the only line that differs from meetXModelsPhi
            phis.push_back(*i);
        }
    }
    return phis;
}

bool PredicateSetDomainLabels::isBottomElement(const PredicateAbstractionLabels &element) const {
    return element.size() == 0;
}

PredicateAbstractionLabels PredicateSetDomainLabels::binary_join(const PredicateAbstractionLabels &e1, const PredicateAbstractionLabels &e2) const {
    if(isBottomElement(e1)) {
        return e2;
    } else if(isBottomElement(e2)) {
        return e1;
    }
    // TODO take advantage of algorithm's std::set_union
    PredicateAbstractionLabels phis = e1; // Make a copy
    for(auto i = e2.cbegin(); i != e2.cend(); i++) {
        if(std::none_of(phis.cbegin(), phis.cend(),
                    [&i](std::optional<SymbolicPredicate> j){ return *i == *j; })) {
            phis.push_back(*i);
        }
    }
    return phis;
}

/**
 * PosteriorDistributionIntervalDomain members
 */

bool PosteriorDistributionIntervalDomainLabels::isBottomElement(const PosteriorDistributionAbstractionLabels &element) const {
    // XXX a better check would be if the intervals for each category
    // actually admit some concretization that forms a probability distribution.
    return element.size() == 0; // This is just based off of the default empty constructor
}

PosteriorDistributionAbstractionLabels PosteriorDistributionIntervalDomainLabels::binary_join(const PosteriorDistributionAbstractionLabels &e1, const PosteriorDistributionAbstractionLabels &e2) const {
    if(isBottomElement(e1)) {
        return e2;
    } else if(isBottomElement(e2)) {
        return e1;
    }
    // XXX strong assumption (invariant?) that the two categorical dist's have the same size
    PosteriorDistributionAbstractionLabels ret(e1.size());
    for(unsigned int i = 0; i < e1.size(); i++) {
        ret[i] = Interval<double>::join(e1[i], e2[i]);
    }
    return ret;
}

/**
 * BoxDropoutDomainLabels members
 */

// First two auxiliary methods

inline bool couldBeEmpty(const TrainingReferencesWithDropoutLabels::DropoutCounts &counts) {
    return std::accumulate(counts.counts.begin(), counts.counts.end(), 0) <= counts.num_dropout;
}

inline bool mustBeEmpty(const TrainingReferencesWithDropoutLabels::DropoutCounts &counts) {
    return std::accumulate(counts.counts.begin(), counts.counts.end(), 0) == 0;
}

void BoxDropoutDomainLabels::computePredicatesAndScores(std::list<ScoreEntry> &exists_nontrivial, std::list<const ScoreEntry *> &forall_nontrivial, const TrainingReferencesWithDropoutLabels &training_set_abstraction, int feature_index) const {
    switch(training_set_abstraction.training_references.getFeatureTypes()[feature_index]) {
        // XXX need to make changes here if adding new feature types
        case FeatureType::BOOLEAN:
            computeBooleanFeaturePredicateAndScore(exists_nontrivial, forall_nontrivial, training_set_abstraction, feature_index);
            break;
        case FeatureType::NUMERIC:
            computeNumericFeaturePredicatesAndScores(exists_nontrivial, forall_nontrivial, training_set_abstraction, feature_index);
            break;
    }
}

void BoxDropoutDomainLabels::computeBooleanFeaturePredicateAndScore(std::list<ScoreEntry> &exists_nontrivial, std::list<const ScoreEntry *> &forall_nontrivial, const TrainingReferencesWithDropoutLabels &training_set_abstraction, int feature_index) const {
    SymbolicPredicate phi(feature_index);
    auto counts = training_set_abstraction.splitCounts(phi);
    if(!mustBeEmpty(counts.first) && !mustBeEmpty(counts.second)) {
        Interval<double> temp = jointImpurityLabels(counts.first.counts,
                                              counts.first.num_dropout,
                                              counts.second.counts,
                                              counts.second.num_dropout);
        exists_nontrivial.push_back(std::make_pair(phi, temp));
        if(!couldBeEmpty(counts.first) && !couldBeEmpty(counts.second)) {
            forall_nontrivial.push_back(&exists_nontrivial.back());
        }
    }
}

void BoxDropoutDomainLabels::computeNumericFeaturePredicatesAndScores(std::list<ScoreEntry> &exists_nontrivial, std::list<const ScoreEntry *> &forall_nontrivial, const TrainingReferencesWithDropoutLabels &training_set_abstraction, int feature_index) const {
    // Largely copy-paste from concrete case
    std::vector<std::pair<float,int>> value_class_pairs(training_set_abstraction.training_references.size());
    for(unsigned int j = 0; j < training_set_abstraction.training_references.size(); j++) {
        DataRow temp = training_set_abstraction.training_references[j];
        value_class_pairs[j].first = temp.x[feature_index].getNumericValue();
        value_class_pairs[j].second = temp.y;
    }
    if(value_class_pairs.size() < 2) {
        return;
    }
    std::sort(value_class_pairs.begin(), value_class_pairs.end(),
              [](const std::pair<float,int> &p1, const std::pair<float,int> &p2)
              { return p1.first < p2.first; } );
    std::pair<TrainingReferencesWithDropoutLabels::DropoutCounts, TrainingReferencesWithDropoutLabels::DropoutCounts> split_counts = {
            { std::vector<int>(training_set_abstraction.training_references.getNumCategories(), 0), 0 },
            { training_set_abstraction.baseCounts(), training_set_abstraction.num_dropout } };
    for(auto i = value_class_pairs.begin(); i + 1 != value_class_pairs.end(); i++) {
        split_counts.first.counts[i->second]++;
        if(split_counts.first.num_dropout < training_set_abstraction.num_dropout) {
            split_counts.first.num_dropout++;
        }
        split_counts.second.counts[i->second]--;
        int remaining = std::accumulate(split_counts.second.counts.cbegin(), split_counts.second.counts.cend(), 0);
        if(remaining < split_counts.second.num_dropout) {
            split_counts.second.num_dropout = remaining;
        }
        if(i->first == (i+1)->first) {
            continue;
        }
        // At this point, the check for if we should include in exists_nontrivial would always pass.
        // For each adjacent pair (l,u) store a symbolic predicate x<=[l,u)
        SymbolicPredicate phi(feature_index, i->first, (i+1)->first);
        Interval<double> temp = jointImpurityLabels(split_counts.first.counts,
                                              split_counts.first.num_dropout,
                                              split_counts.second.counts,
                                              split_counts.second.num_dropout);
        exists_nontrivial.push_back(std::make_pair(phi, temp));
        if(!couldBeEmpty(split_counts.first) && !couldBeEmpty(split_counts.second)) {
            forall_nontrivial.push_back(&exists_nontrivial.back());
        }
    }
}

PredicateAbstractionLabels BoxDropoutDomainLabels::bestSplit(const TrainingReferencesWithDropoutLabels &training_set_abstraction) const {
    std::list<ScoreEntry> exists_nontrivial;
    std::list<const ScoreEntry *> forall_nontrivial; // Points to elements in exists_nontrivial
    for(int i = 0; i < training_set_abstraction.training_references.getFeatureTypes().size(); i++) {
        computePredicatesAndScores(exists_nontrivial, forall_nontrivial, training_set_abstraction, i);
    }

    if(forall_nontrivial.size() == 0) {
        PredicateAbstractionLabels ret(exists_nontrivial.size() + 1);
        int index = 0;
        for(auto i = exists_nontrivial.cbegin(); i != exists_nontrivial.cend(); i++,index++) {
            ret[index] = i->first;
        }
        ret[exists_nontrivial.size()] = {}; // The non option type value
        return ret;
    } else {
        // Find the threshold using only predicates from forall_nontrivial
        double min_upper_bound; // Always gets initialized unless forall_nontrivial.size() == 0,
                                // and we prior have an if statement to check that
        for(auto i = forall_nontrivial.cbegin(); i != forall_nontrivial.cend(); i++) {
            if(i == forall_nontrivial.begin() || min_upper_bound > (*i)->second.get_upper_bound()) {
                min_upper_bound = (*i)->second.get_upper_bound();
            }
        }
        // Return any predicates in exists_nontrivial whose score could beat the threshold
        PredicateAbstractionLabels ret;
        for(auto i = exists_nontrivial.cbegin(); i != exists_nontrivial.cend(); i++) {
            if(i->second.get_lower_bound() <= min_upper_bound) {
                ret.push_back(i->first);
            }
        }
        return ret;
    }
}

TrainingReferencesWithDropoutLabels BoxDropoutDomainLabels::filter(const TrainingReferencesWithDropoutLabels &training_set_abstraction, const PredicateAbstractionLabels &predicate_abstraction) const {
    std::vector<TrainingReferencesWithDropoutLabels> joins;
    for(auto i = predicate_abstraction.cbegin(); i != predicate_abstraction.cend(); i++) {
        // The grammar should enforce that each i->has_value()
        TrainingReferencesWithDropoutLabels temp = training_set_abstraction.filter(i->value(), true);
        joins.push_back(temp);
    }
    return training_set_domain->join(joins);
}

TrainingReferencesWithDropoutLabels BoxDropoutDomainLabels::filterNegated(const TrainingReferencesWithDropoutLabels &training_set_abstraction, const PredicateAbstractionLabels &predicate_abstraction) const {
    // XXX copy and pasted previous method with one change; refactor with common method
    std::vector<TrainingReferencesWithDropoutLabels> joins;
    for(auto i = predicate_abstraction.cbegin(); i != predicate_abstraction.cend(); i++) {
        // The grammar should enforce that each i->has_value()
        TrainingReferencesWithDropoutLabels temp = training_set_abstraction.filter(i->value(), false);
        joins.push_back(temp);
    }
    return training_set_domain->join(joins);
}

PosteriorDistributionAbstractionLabels BoxDropoutDomainLabels::summary(const TrainingReferencesWithDropoutLabels &training_set_abstraction) const {
    return estimateCategoricalLabels(training_set_abstraction.baseCounts(), training_set_abstraction.num_dropout);
}
