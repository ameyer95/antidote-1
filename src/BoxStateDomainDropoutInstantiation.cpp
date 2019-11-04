#include "BoxStateDomainDropoutInstantiation.h"
#include "Feature.hpp"
#include "information_math.h"
#include "Interval.h"
#include <algorithm>
#include <list>
#include <numeric> // for std::accumulate
#include <unordered_map>
#include <utility>
#include <vector>

/**
 * TrainingReferencesWithDropout members
 */

TrainingReferencesWithDropout::TrainingReferencesWithDropout(DataReferences training_references, int num_dropout) {
    this->training_references = training_references;
    this->num_dropout = num_dropout;
}

std::vector<int> TrainingReferencesWithDropout::baseCounts() const {
    std::vector<int> counts(training_references.getNumCategories(), 0);
    for(unsigned int i = 0; i < training_references.size(); i++) {
        counts[training_references[i].y]++;
    }
    return counts;
}

std::list<SymbolicPredicate> TrainingReferencesWithDropout::gatherPredicates() const {
    std::list<SymbolicPredicate> ret;
    FeatureVectorHeader header = training_references.getFeatureTypes();
    for(unsigned int i = 0; i < header.size(); i++) {
        switch(header[i]) {
            // XXX need to make changes here if adding new feature types
            case FeatureType::BOOLEAN:
                ret.push_back(SymbolicPredicate(i));
                break;
            case FeatureType::NUMERIC:
                // XXX TODO need to implement numeric case
                break;
        }
    }
    return ret;
}

std::pair<TrainingReferencesWithDropout::DropoutCounts, TrainingReferencesWithDropout::DropoutCounts> TrainingReferencesWithDropout::splitCounts(const SymbolicPredicate &phi) const {
    std::pair<DropoutCounts, DropoutCounts> ret;
    ret.first.counts = std::vector<int>(training_references.getNumCategories(), 0);
    ret.second.counts = std::vector<int>(training_references.getNumCategories(), 0);
    DropoutCounts *d_ptr;
    for(unsigned int i = 0; i < training_references.size(); i++) {
        std::optional<bool> result = phi.evaluate(training_references[i].x);
        int category = training_references[i].y;
        // XXX TODO strong assumption that result.has_value() --- need to be more general for numeric case
        d_ptr = result.value() ? &(ret.second) : &(ret.first);
        d_ptr->counts[category]++;
    }
    // Ensure num_dropouts are well-defined XXX this is more complicated in the 3-valued case
    std::vector<DropoutCounts*> iters = {&(ret.first), &(ret.second)};
    for(auto i = iters.begin(); i != iters.end(); i++) {
        (*i)->num_dropout = std::min(num_dropout, std::accumulate((*i)->counts.cbegin(), (*i)->counts.cend(), 0));
    }
    return ret;
}

TrainingReferencesWithDropout TrainingReferencesWithDropout::pureSetRestriction(std::list<int> pure_possible_classes) const {
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
    return TrainingReferencesWithDropout(training_copy, num_dropout - num_removed);
}

TrainingReferencesWithDropout TrainingReferencesWithDropout::filter(const SymbolicPredicate &phi, bool positive_flag) const {
    TrainingReferencesWithDropout ret(*this);
    bool remove, result;
    for(unsigned int i = 0; i < ret.training_references.size(); i++) {
        // XXX TODO we're not yet handling numeric cases
        result = phi.evaluate(ret.training_references[i].x).value();
        remove = (positive_flag != result);
        if(remove) {
            ret.training_references.remove(i);
            i--;
        }
    }
    if(ret.num_dropout > ret.training_references.size()) {
        ret.num_dropout = ret.training_references.size();
    }
    return ret;
}

/**
 * TrainingSetDropoutDomain members
 */

TrainingReferencesWithDropout TrainingSetDropoutDomain::meetImpurityEqualsZero(const TrainingReferencesWithDropout &element) const {
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
        return TrainingReferencesWithDropout();
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

TrainingReferencesWithDropout TrainingSetDropoutDomain::meetImpurityNotEqualsZero(const TrainingReferencesWithDropout &element) const {
    // Our abstraction is not capable of expressing this precisely
    return element;
}

bool TrainingSetDropoutDomain::isBottomElement(const TrainingReferencesWithDropout &element) const {
    return element.training_references.size() == 0;
}

TrainingReferencesWithDropout TrainingSetDropoutDomain::binary_join(const TrainingReferencesWithDropout &e1, const TrainingReferencesWithDropout &e2) const {
    if(isBottomElement(e1)) {
        return e2;
    } else if(isBottomElement(e2)) {
        return e1;
    }
    DataReferences d = DataReferences::set_union(e1.training_references, e2.training_references);
    int n1 = d.size() - e2.training_references.size() + e2.num_dropout; // Note |(T1 U T2) \ T1| = |T2 \ T1|
    int n2 = d.size() - e1.training_references.size() + e1.num_dropout;
    return TrainingReferencesWithDropout(d, std::max(n1, n2));
}

/**
 * PredicateSetDomain members
 */

PredicateAbstraction PredicateSetDomain::meetPhiIsBottom(const PredicateAbstraction &element) const {
    bool contains_bottom = std::any_of(element.cbegin(), element.cend(),
            [](std::optional<SymbolicPredicate> phi){ return !phi.has_value(); });
    if(contains_bottom) {
        std::optional<SymbolicPredicate> bottom = {};
        return PredicateAbstraction({bottom}); // The vector of a single bottom element.
    } else {
        return PredicateAbstraction(); // The empty vector
    }
}

PredicateAbstraction PredicateSetDomain::meetPhiIsNotBottom(const PredicateAbstraction &element) const {
    PredicateAbstraction phis(element); // Make a copy
    // Iterate over the copy and delete any bottom element
    for(auto i = phis.begin(); i != phis.end(); i++) {
        if(!i->has_value()) { // It should be an invariant of the code that this only happens once, but...
            phis.erase(i--);
        }
    }
    return phis;
}

PredicateAbstraction PredicateSetDomain::meetXModelsPhi(const PredicateAbstraction &element) const {
    if(isBottomElement(element)) {
        return element;
    }
    PredicateAbstraction phis;
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

PredicateAbstraction PredicateSetDomain::meetXNotModelsPhi(const PredicateAbstraction &element) const {
    if(isBottomElement(element)) {
        return element;
    }
    PredicateAbstraction phis;
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

bool PredicateSetDomain::isBottomElement(const PredicateAbstraction &element) const {
    return element.size() == 0;
}

PredicateAbstraction PredicateSetDomain::binary_join(const PredicateAbstraction &e1, const PredicateAbstraction &e2) const {
    if(isBottomElement(e1)) {
        return e2;
    } else if(isBottomElement(e2)) {
        return e1;
    }
    // TODO take advantage of algorithm's std::set_union
    PredicateAbstraction phis = e1; // Make a copy
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

bool PosteriorDistributionIntervalDomain::isBottomElement(const PosteriorDistributionAbstraction &element) const {
    // XXX a better check would be if the intervals for each category
    // actually admit some concretization that forms a probability distribution.
    return element.size() == 0; // This is just based off of the default empty constructor
}

PosteriorDistributionAbstraction PosteriorDistributionIntervalDomain::binary_join(const PosteriorDistributionAbstraction &e1, const PosteriorDistributionAbstraction &e2) const {
    // XXX strong assumption (invariant?) that the two categorical dist's have the same size
    PosteriorDistributionAbstraction ret(e1.size());
    for(unsigned int i = 0; i < e1.size(); i++) {
        ret[i] = Interval<double>::join(e1[i], e2[i]);
    }
    return ret;
}

/**
 * BoxDropoutDomain members
 */

// First two auxiliary methods

bool couldBeEmpty(const TrainingReferencesWithDropout::DropoutCounts &counts) {
    return std::accumulate(counts.counts.begin(), counts.counts.end(), 0) <= counts.num_dropout;
}

bool mustBeEmpty(const TrainingReferencesWithDropout::DropoutCounts &counts) {
    return std::accumulate(counts.counts.begin(), counts.counts.end(), 0) == 0;
}

// And a wrapper for SymbolicPredicate::hash so we can conveniently use std::unordered_map
struct hash_SymbolicPredicate {
    size_t operator()(const SymbolicPredicate &p) const {
        return p.hash();
    }
};

PredicateAbstraction BoxDropoutDomain::bestSplit(const TrainingReferencesWithDropout &training_set_abstraction) const {
    std::list<SymbolicPredicate> candidates = training_set_abstraction.gatherPredicates();
    std::unordered_map<const SymbolicPredicate, std::pair<TrainingReferencesWithDropout::DropoutCounts, TrainingReferencesWithDropout::DropoutCounts>, hash_SymbolicPredicate> counts;
    PredicateAbstraction forall_nontrivial, exists_nontrivial;

    for(auto i = candidates.cbegin(); i != candidates.cend(); i++) {
        counts.insert(std::make_pair(*i, training_set_abstraction.splitCounts(*i)));
        if(!couldBeEmpty(counts[*i].first) && !couldBeEmpty(counts[*i].second)) {
            forall_nontrivial.push_back(*i);
        }
        if(!mustBeEmpty(counts[*i].first) && !mustBeEmpty(counts[*i].second)) {
            exists_nontrivial.push_back(*i);
        }
    }

    if(forall_nontrivial.size() == 0) {
        exists_nontrivial.push_back({}); // The none option type value
        return exists_nontrivial;
    }

    // Compute and store all of the predicates' scores
    std::unordered_map<const SymbolicPredicate, Interval<double>, hash_SymbolicPredicate> scores;
    for(auto i = exists_nontrivial.cbegin(); i != exists_nontrivial.cend(); i++) {
        SymbolicPredicate index = i->value(); // Prior for-loop guarantees i->has_value()
        Interval<double> temp = jointImpurity(counts[index].first.counts,
                                              counts[index].first.num_dropout,
                                              counts[index].second.counts,
                                              counts[index].second.num_dropout);
        scores.insert(std::make_pair(index, temp));
    }

    // Find the threshold using only predicates from forall_nontrivial
    double min_upper_bound; // Always gets initialized unless forall_nontrivial.size() == 0,
                            // and we prior have an if statement to check that
    for(auto i = forall_nontrivial.cbegin(); i != forall_nontrivial.cend(); i++) {
        SymbolicPredicate index = i->value(); // Again, guaranteed i->has_value()
        if(i == forall_nontrivial.begin() || min_upper_bound > scores[index].get_upper_bound()) {
            min_upper_bound = scores[index].get_upper_bound();
        }
    }

    // Return any predicates in exists_nontrivial whose score could beat the threshold
    PredicateAbstraction ret;
    for(auto i = exists_nontrivial.cbegin(); i != exists_nontrivial.cend(); i++) {
        SymbolicPredicate index = i->value(); // Again again, guaranteed i->has_value()
        if(scores[index].get_lower_bound() <= min_upper_bound) {
            ret.push_back(*i);
        }
    }

    return ret;
}

TrainingReferencesWithDropout BoxDropoutDomain::filter(const TrainingReferencesWithDropout &training_set_abstraction, const PredicateAbstraction &predicate_abstraction) const {
    std::vector<TrainingReferencesWithDropout> joins;
    for(auto i = predicate_abstraction.cbegin(); i != predicate_abstraction.cend(); i++) {
        // The grammar should enforce that each i->has_value()
        TrainingReferencesWithDropout temp = training_set_abstraction.filter(i->value(), true);
        joins.push_back(temp);
    }
    return training_set_domain->join(joins);
}

TrainingReferencesWithDropout BoxDropoutDomain::filterNegated(const TrainingReferencesWithDropout &training_set_abstraction, const PredicateAbstraction &predicate_abstraction) const {
    // XXX copy and pasted previous method with one change; refactor with common method
    std::vector<TrainingReferencesWithDropout> joins;
    for(auto i = predicate_abstraction.cbegin(); i != predicate_abstraction.cend(); i++) {
        // The grammar should enforce that each i->has_value()
        TrainingReferencesWithDropout temp = training_set_abstraction.filter(i->value(), false);
        joins.push_back(temp);
    }
    return training_set_domain->join(joins);
}

PosteriorDistributionAbstraction BoxDropoutDomain::summary(const TrainingReferencesWithDropout &training_set_abstraction) const {
    return estimateCategorical(training_set_abstraction.baseCounts(), training_set_abstraction.num_dropout);
}
