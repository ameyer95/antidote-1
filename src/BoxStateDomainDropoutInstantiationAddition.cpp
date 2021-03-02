#include "BoxStateDomainDropoutInstantiationAddition.h"
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

TrainingReferencesWithDropoutAddition::TrainingReferencesWithDropoutAddition(DataReferences training_references, int num_dropout, int sensitive_feature, float protected_value) {
    this->training_references = training_references;
    this->num_dropout = num_dropout;
    this->sensitive_feature = sensitive_feature;
    this->protected_value = protected_value; // the value of the sensitive attribute that is protected

}

std::vector<int> TrainingReferencesWithDropoutAddition::baseCounts() const {
    std::vector<int> counts(training_references.getNumCategories(), 0);
    for(unsigned int i = 0; i < training_references.size(); i++) {
        counts[training_references[i].y]++;
    }
    return counts;
}

std::pair<TrainingReferencesWithDropoutAddition::DropoutCountsWithProtected, TrainingReferencesWithDropoutAddition::DropoutCountsWithProtected> TrainingReferencesWithDropoutAddition::splitCounts(const SymbolicPredicate &phi) const {
    std::pair<DropoutCountsWithProtected, DropoutCountsWithProtected> ret;
    // Vector syntax: initialize a vector of length num categories to be all 0's
    ret.first.dropout.counts = std::vector<int>(training_references.getNumCategories(), 0);
    ret.second.dropout.counts = std::vector<int>(training_references.getNumCategories(), 0);
    ret.first.protected_counts = std::vector<int>(training_references.getNumCategories(), 0);
    ret.first.protected_counts = std::vector<int>(training_references.getNumCategories(), 0);

    DropoutCountsWithProtected *d_ptr;

    for(unsigned int i = 0; i < training_references.size(); i++) {
        std::optional<bool> result = phi.evaluate(training_references[i].x);
        int category = training_references[i].y;
        // XXX it should be an invariant of the way predicates are selected
        // that we always have result.has_value()
        // but this could be done more safely
        d_ptr = result.value() ? &(ret.second) : &(ret.first);
        d_ptr->dropout.counts[category]++;

        if (training_references[i].x[sensitive_feature].getNumericValue() == protected_value) {
            d_ptr->protected_counts[category]++;
        }
    }
    // Ensure num_dropouts are well-defined XXX this is more complicated in the 3-valued case
    // except that it's not---again, there should be an invariant that we never fall into
    // maybe cases in this portion of the code
    std::vector<DropoutCounts*> iters = {&(ret.first.dropout), &(ret.second.dropout)};
    for(auto i = iters.begin(); i != iters.end(); i++) {
        (*i)->num_dropout = std::min(num_dropout, std::accumulate((*i)->counts.cbegin(), (*i)->counts.cend(), 0));
    }
    return ret;
}

TrainingReferencesWithDropoutAddition TrainingReferencesWithDropoutAddition::pureSetRestriction(std::list<int> pure_possible_classes) const {
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
    return TrainingReferencesWithDropoutAddition(training_copy, num_dropout - num_removed, sensitive_feature, protected_value);
}

TrainingReferencesWithDropoutAddition TrainingReferencesWithDropoutAddition::filter(const SymbolicPredicate &phi, bool positive_flag) const {
    TrainingReferencesWithDropoutAddition ret(*this);
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
 * TrainingSetDropoutDomainAddition members
 */

TrainingReferencesWithDropoutAddition TrainingSetDropoutDomainAddition::meetImpurityEqualsZero(const TrainingReferencesWithDropoutAddition &element) const {
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
        return TrainingReferencesWithDropoutAddition();
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

TrainingReferencesWithDropoutAddition TrainingSetDropoutDomainAddition::meetImpurityNotEqualsZero(const TrainingReferencesWithDropoutAddition &element) const {
    // Our abstraction is not capable of expressing this precisely
    return element;
}

bool TrainingSetDropoutDomainAddition::isBottomElement(const TrainingReferencesWithDropoutAddition &element) const {
    return element.training_references.size() == 0;
}

TrainingReferencesWithDropoutAddition TrainingSetDropoutDomainAddition::binary_join(const TrainingReferencesWithDropoutAddition &e1, const TrainingReferencesWithDropoutAddition &e2) const {
    if(isBottomElement(e1)) {
        return e2;
    } else if(isBottomElement(e2)) {
        return e1;
    }
    DataReferences d = DataReferences::set_union(e1.training_references, e2.training_references);
    int n1 = d.size() - e2.training_references.size() + e2.num_dropout; // Note |(T1 U T2) \ T1| = |T2 \ T1|
    int n2 = d.size() - e1.training_references.size() + e1.num_dropout;
    return TrainingReferencesWithDropoutAddition(d, std::max(n1, n2), e1.sensitive_feature, e1.protected_value);
}

/**
 * PredicateSetDomain members
 */

PredicateAbstractionAddition PredicateSetDomainAddition::meetPhiIsBottom(const PredicateAbstractionAddition &element) const {
    bool contains_bottom = std::any_of(element.cbegin(), element.cend(),
            [](std::optional<SymbolicPredicate> phi){ return !phi.has_value(); });
    if(contains_bottom) {
        std::optional<SymbolicPredicate> bottom = {};
        return PredicateAbstractionAddition({bottom}); // The vector of a single bottom element.
    } else {
        return PredicateAbstractionAddition(); // The empty vector
    }
}

PredicateAbstractionAddition PredicateSetDomainAddition::meetPhiIsNotBottom(const PredicateAbstractionAddition &element) const {
    PredicateAbstractionAddition phis(element); // Make a copy
    // Iterate over the copy and delete any bottom element
    for(auto i = phis.begin(); i != phis.end(); i++) {
        if(!i->has_value()) { // It should be an invariant of the code that this only happens once, but...
            phis.erase(i--);
        }
    }
    return phis;
}

PredicateAbstractionAddition PredicateSetDomainAddition::meetXModelsPhi(const PredicateAbstractionAddition &element, const FeatureVector &x) const {
    if(isBottomElement(element)) {
        return element;
    }
    PredicateAbstractionAddition phis;
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

PredicateAbstractionAddition PredicateSetDomainAddition::meetXNotModelsPhi(const PredicateAbstractionAddition &element, const FeatureVector &x) const {
    if(isBottomElement(element)) {
        return element;
    }
    PredicateAbstractionAddition phis;
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

bool PredicateSetDomainAddition::isBottomElement(const PredicateAbstractionAddition &element) const {
    return element.size() == 0;
}

PredicateAbstractionAddition PredicateSetDomainAddition::binary_join(const PredicateAbstractionAddition &e1, const PredicateAbstractionAddition &e2) const {
    if(isBottomElement(e1)) {
        return e2;
    } else if(isBottomElement(e2)) {
        return e1;
    }
    // TODO take advantage of algorithm's std::set_union
    PredicateAbstractionAddition phis = e1; // Make a copy
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

bool PosteriorDistributionIntervalDomainAddition::isBottomElement(const PosteriorDistributionAbstractionAddition &element) const {
    // XXX a better check would be if the intervals for each category
    // actually admit some concretization that forms a probability distribution.
    return element.size() == 0; // This is just based off of the default empty constructor
}

PosteriorDistributionAbstractionAddition PosteriorDistributionIntervalDomainAddition::binary_join(const PosteriorDistributionAbstractionAddition &e1, const PosteriorDistributionAbstractionAddition &e2) const {
    if(isBottomElement(e1)) {
        return e2;
    } else if(isBottomElement(e2)) {
        return e1;
    }
    // XXX strong assumption (invariant?) that the two categorical dist's have the same size
    PosteriorDistributionAbstractionAddition ret(e1.size());
    for(unsigned int i = 0; i < e1.size(); i++) {
        ret[i] = Interval<double>::join(e1[i], e2[i]);
    }
    return ret;
}

/**
 * BoxDropoutDomainAddition members
 */

// First two auxiliary methods

inline bool couldBeEmpty(const TrainingReferencesWithDropoutAddition::DropoutCounts &counts) {
    return std::accumulate(counts.counts.begin(), counts.counts.end(), 0) <= counts.num_dropout;
}

inline bool mustBeEmpty(const TrainingReferencesWithDropoutAddition::DropoutCounts &counts) {
    return std::accumulate(counts.counts.begin(), counts.counts.end(), 0) == 0;
}

void BoxDropoutDomainAddition::computePredicatesAndScores(std::list<ScoreEntry> &exists_nontrivial, std::list<const ScoreEntry *> &forall_nontrivial, const TrainingReferencesWithDropoutAddition &training_set_abstraction, int feature_index) const {
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

void BoxDropoutDomainAddition::computeBooleanFeaturePredicateAndScore(std::list<ScoreEntry> &exists_nontrivial, std::list<const ScoreEntry *> &forall_nontrivial, const TrainingReferencesWithDropoutAddition &training_set_abstraction, int feature_index) const {
    SymbolicPredicate phi(feature_index);
    auto counts = training_set_abstraction.splitCounts(phi);
    if(!mustBeEmpty(counts.first.dropout) && !mustBeEmpty(counts.second.dropout)) {
        Interval<double> temp;
        // TO DO ANNA - think about this; what does counting the protected group accomplish?
        if (training_set_abstraction.sensitive_feature > -1) {
            temp = jointImpurityAdditionLop(counts.first.dropout.counts,
                                                counts.first.dropout.num_dropout,
                                                counts.second.dropout.counts,
                                                counts.second.dropout.num_dropout);
        }
        else {
            temp = jointImpurityAddition(counts.first.dropout.counts,
                                              counts.first.dropout.num_dropout,
                                              counts.second.dropout.counts,
                                              counts.second.dropout.num_dropout);
        }

        exists_nontrivial.push_back(std::make_pair(phi, temp));
        if(!couldBeEmpty(counts.first.dropout) && !couldBeEmpty(counts.second.dropout)) {
            forall_nontrivial.push_back(&exists_nontrivial.back());
        }
    }
}

void BoxDropoutDomainAddition::computeNumericFeaturePredicatesAndScores(std::list<ScoreEntry> &exists_nontrivial, std::list<const ScoreEntry *> &forall_nontrivial, const TrainingReferencesWithDropoutAddition &training_set_abstraction, int feature_index) const {
    // Largely copy-paste from concrete case
    std::vector<std::pair<float,int>> value_class_pairs(training_set_abstraction.training_references.size());
    std::vector<std::pair<float,bool>> protected_status_pairs(training_set_abstraction.training_references.size());
    std::vector<int> protected_counts(training_set_abstraction.training_references.getNumCategories(), 0);
    int num_we_can_flip = 0;

    for(unsigned int j = 0; j < training_set_abstraction.training_references.size(); j++) {
        DataRow temp = training_set_abstraction.training_references[j];
        value_class_pairs[j].first = temp.x[feature_index].getNumericValue();
        value_class_pairs[j].second = temp.y;

        if (training_set_abstraction.sensitive_feature > -1) {
            protected_status_pairs[j].first = temp.x[feature_index].getNumericValue();
            protected_status_pairs[j].second = temp.x[training_set_abstraction.sensitive_feature].getNumericValue() == training_set_abstraction.protected_value;
            if (temp.x[training_set_abstraction.sensitive_feature].getNumericValue() == training_set_abstraction.protected_value) {
                if (temp.y == 0) {
                    num_we_can_flip++; // increment number we can flip for protected entry with label 0
                }
                protected_counts[temp.y]++;
            }
        }
    }

    if(value_class_pairs.size() < 2) {
        return;
    }
    int max_labels_to_flip = (training_set_abstraction.sensitive_feature > -1 ) ? 
                            std::min(num_we_can_flip, training_set_abstraction.num_dropout) : 
                            training_set_abstraction.num_dropout;
    std::sort(value_class_pairs.begin(), value_class_pairs.end(),
              [](const std::pair<float,int> &p1, const std::pair<float,int> &p2)
              { return p1.first < p2.first; } );
    if (training_set_abstraction.sensitive_feature > -1) {
        std::sort(protected_status_pairs.begin(), protected_status_pairs.end(),
                    [](const std::pair<float,bool> &p1, const std::pair<float,bool> &p2)
                    { return p1.first < p2.first; } );
    }
    
    std::pair<TrainingReferencesWithDropoutAddition::DropoutCountsWithProtected, TrainingReferencesWithDropoutAddition::DropoutCountsWithProtected> split_counts = {
            { {std::vector<int>(training_set_abstraction.training_references.getNumCategories(), 0), 0},
               std::vector<int>(training_set_abstraction.training_references.getNumCategories(), 0) },
            { {training_set_abstraction.baseCounts(), max_labels_to_flip}, 
               protected_counts } };
    int counter=-1;
    for(auto i = value_class_pairs.begin(); i + 1 != value_class_pairs.end(); i++) {
        counter++;
        split_counts.first.dropout.counts[i->second]++;
        split_counts.second.dropout.counts[i->second]--;

        if (training_set_abstraction.sensitive_feature > -1 && protected_status_pairs[counter].second) {
            split_counts.first.protected_counts[i->second]++;
            split_counts.second.protected_counts[i->second]--;
        }

        if(split_counts.first.dropout.num_dropout < max_labels_to_flip) {
            split_counts.first.dropout.num_dropout++;
        }
        
        int remaining = std::accumulate(split_counts.second.dropout.counts.cbegin(), split_counts.second.dropout.counts.cend(), 0);
        if(remaining < split_counts.second.dropout.num_dropout) {
            split_counts.second.dropout.num_dropout = remaining;
        }

        // Next, if i and i+1 have distinct float values, we'll consider a predicate here
        if(i->first == (i+1)->first) {
            continue;
        }
        // At this point, the check for if we should include in exists_nontrivial would always pass.
        // For each adjacent pair (l,u) store a symbolic predicate x<=[l,u)
        SymbolicPredicate phi(feature_index, i->first, (i+1)->first);
        Interval<double> temp;
        if (training_set_abstraction.sensitive_feature > -1) {
            temp = jointImpurityAdditionLop(split_counts.first.dropout.counts,
                                                split_counts.first.dropout.num_dropout,
                                                split_counts.second.dropout.counts,
                                                split_counts.second.dropout.num_dropout);
        }
        else {
            temp = jointImpurityAddition(split_counts.first.dropout.counts,
                                              split_counts.first.dropout.num_dropout,
                                              split_counts.second.dropout.counts,
                                              split_counts.second.dropout.num_dropout);
        }
        exists_nontrivial.push_back(std::make_pair(phi, temp));
        if(!couldBeEmpty(split_counts.first.dropout) && !couldBeEmpty(split_counts.second.dropout)) {
            forall_nontrivial.push_back(&exists_nontrivial.back());
        }
    }
}

PredicateAbstractionAddition BoxDropoutDomainAddition::bestSplit(const TrainingReferencesWithDropoutAddition &training_set_abstraction) const {
    std::list<ScoreEntry> exists_nontrivial;
    std::list<const ScoreEntry *> forall_nontrivial; // Points to elements in exists_nontrivial
    for(int i = 0; i < training_set_abstraction.training_references.getFeatureTypes().size(); i++) {
        computePredicatesAndScores(exists_nontrivial, forall_nontrivial, training_set_abstraction, i);
    }

    if(forall_nontrivial.size() == 0) {
        PredicateAbstractionAddition ret(exists_nontrivial.size() + 1);
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
        PredicateAbstractionAddition ret;
        for(auto i = exists_nontrivial.cbegin(); i != exists_nontrivial.cend(); i++) {
            if(i->second.get_lower_bound() <= min_upper_bound) {
                ret.push_back(i->first);
            }
        }
        return ret;
    }
}

TrainingReferencesWithDropoutAddition BoxDropoutDomainAddition::filter(const TrainingReferencesWithDropoutAddition &training_set_abstraction, const PredicateAbstractionAddition &predicate_abstraction) const {
    std::vector<TrainingReferencesWithDropoutAddition> joins;
    for(auto i = predicate_abstraction.cbegin(); i != predicate_abstraction.cend(); i++) {
        // The grammar should enforce that each i->has_value()
        TrainingReferencesWithDropoutAddition temp = training_set_abstraction.filter(i->value(), true);
        joins.push_back(temp);
    }
    return training_set_domain->join(joins);
}

TrainingReferencesWithDropoutAddition BoxDropoutDomainAddition::filterNegated(const TrainingReferencesWithDropoutAddition &training_set_abstraction, const PredicateAbstractionAddition &predicate_abstraction) const {
    // XXX copy and pasted previous method with one change; refactor with common method
    std::vector<TrainingReferencesWithDropoutAddition> joins;
    for(auto i = predicate_abstraction.cbegin(); i != predicate_abstraction.cend(); i++) {
        // The grammar should enforce that each i->has_value()
        TrainingReferencesWithDropoutAddition temp = training_set_abstraction.filter(i->value(), false);
        joins.push_back(temp);
    }
    return training_set_domain->join(joins);
}

PosteriorDistributionAbstractionAddition BoxDropoutDomainAddition::summary(const TrainingReferencesWithDropoutAddition&training_set_abstraction) const {
    return estimateCategoricalAddition(training_set_abstraction.baseCounts(), training_set_abstraction.num_dropout);
}
