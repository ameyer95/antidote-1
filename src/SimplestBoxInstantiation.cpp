#include "SimplestBoxInstantiation.h"
#include "information_math.h"
#include "Interval.h"
#include <algorithm> // for std::max
#include <map>
#include <optional>
#include <utility>
#include <vector>

/**
 * Some common primitives for the training-set-with-n-elements-missing
 * (more generally, its member functions...)
 */

BooleanDropoutSet::BooleanDropoutSet() {
    bottom_element_flag = true;
}

BooleanDropoutSet::BooleanDropoutSet(DataReferences<BooleanXYPair> training_set, int num_dropout) {
    this->training_set = training_set;
    this->num_dropout = num_dropout;
    bottom_element_flag = false; // XXX this is only correct when the passed parameters behave nicely
}

std::pair<int, int> BooleanDropoutSet::baseCounts() const {
    std::pair<int, int> counts;
    for(int i = 0; i < training_set.size(); i++) {
        if(training_set[i].second) {
            counts.second++;
        } else {
            counts.first++;
        }
    }
    return counts;
}

std::pair<BooleanDropoutSet::DropoutCounts, BooleanDropoutSet::DropoutCounts> BooleanDropoutSet::splitCounts(int bit_index) const {
    std::pair<BooleanDropoutSet::DropoutCounts, BooleanDropoutSet::DropoutCounts> ret({0, 0, 0}, {0, 0, 0});
    BooleanDropoutSet::DropoutCounts *d_ptr;
    int *count_ptr;
    for(unsigned int i = 0; i < training_set.size(); i++) {
        d_ptr = training_set[i].first[bit_index] ? &(ret.second) : &(ret.first);
        count_ptr = training_set[i].second ? &(d_ptr->pos) : &(d_ptr->neg);
        *count_ptr += 1;
    }
    ret.first.num_dropout = std::min(num_dropout, ret.first.pos + ret.first.neg);
    ret.second.num_dropout = std::min(num_dropout, ret.second.pos + ret.second.neg);
    return ret;
};

BooleanDropoutSet BooleanDropoutSet::pureSet(bool classification) const {
    DataReferences<BooleanXYPair> training_set_copy = training_set;
    int num_removed = 0;
    for(unsigned int i = 0; i < training_set_copy.size(); i++) {
        if(training_set_copy[i].second != classification) {
            training_set_copy.remove(i);
            num_removed++;
            i--;
        }
    }
    // We will only call this when it's guaranteed to be non-trivial,
    // so we need not check that num_removed <= num_dropout
    return BooleanDropoutSet(training_set_copy, num_dropout - num_removed);
}

BooleanDropoutSet BooleanDropoutSet::filter(int bit_index, bool positive_flag) const {
    BooleanDropoutSet ret(*this);
    bool remove, result;
    for(unsigned int i = 0; i < ret.training_set.size(); i++) {
        result = ret.training_set[i].first[bit_index];
        remove = (positive_flag != result);
        if(remove) {
            ret.training_set.remove(i);
            i--;
        }
    }
    if(ret.num_dropout > ret.training_set.size()) {
        ret.num_dropout = ret.training_set.size();
    }
}

/**
 * BitvectorPredicateAbstraction members
 */

BitvectorPredicateAbstraction::BitvectorPredicateAbstraction() {
    bottom_element_flag = true;
}

BitvectorPredicateAbstraction::BitvectorPredicateAbstraction(const std::vector<std::optional<int>> &predicates) {
    this->predicates = predicates;
    bottom_element_flag = this->predicates.size() > 0; // not a bottom element when there are non-zero predicates
}

/**
 * Constituent domain member functions
 */

BooleanDropoutSet BooleanDropoutDomain::meetImpurityEqualsZero(const BooleanDropoutSet &element) const {
    if(element.isBottomElement()) {
        return element;
    }
    std::pair<int, int> counts = element.baseCounts();
    bool pure_0_possible = false, pure_1_possible = false;
    if(counts.first <= element.num_dropout) {
        // It's possible that all of the 0-classification elements could be removed
        pure_1_possible = true;
    }
    if(counts.second <= element.num_dropout) {
        pure_0_possible = true;
    }

    // If neither are possible, we return a bottom element
    if(!pure_0_possible && !pure_1_possible) {
        return BooleanDropoutSet();
    }
    // If both are possible, our abstraction is too coarse to do anything precise,
    // so we will overapproximate the meet by returning the current value
    if(pure_0_possible && pure_1_possible) {
        return element;
    }
    // If only one is possible, we restrict the base training set accordingly (with an updated num_dropout)
    if(pure_0_possible) {
        return element.pureSet(false);
    } else { // when pure_1_possible
        return element.pureSet(true);
    }
}

BooleanDropoutSet BooleanDropoutDomain::meetImpurityNotEqualsZero(const BooleanDropoutSet &element) const {
    // Our abstraction is not capable of expressing this precisely
    return element;
}

BooleanDropoutSet BooleanDropoutDomain::binary_join(const BooleanDropoutSet &e1, const BooleanDropoutSet &e2) const {
    if(e1.isBottomElement()) {
        return e2;
    } else if(e2.isBottomElement()) {
        return e1;
    }
    DataReferences<BooleanXYPair> d = DataReferences<BooleanXYPair>::set_union(e1.training_set, e2.training_set);
    int n1 = d.size() - e2.training_set.size() + e2.num_dropout; // Note |(T1 U T2) \ T1| = |T2 \ T1|
    int n2 = d.size() - e1.training_set.size() + e1.num_dropout;
    return BooleanDropoutSet(d, std::max(n1, n2));
}

void BitvectorPredicateDomain::setExecutionDetails(int num_X_indices, const std::vector<bool> &x) {
    this->num_X_indices = num_X_indices;
    this->x = x;
}

BitvectorPredicateAbstraction BitvectorPredicateDomain::abstractBottomPhi() const {
    std::optional<int> bottom = {};
    return BitvectorPredicateAbstraction({bottom});
}

BitvectorPredicateAbstraction BitvectorPredicateDomain::abstractNotBottomPhi() const {
    std::vector<std::optional<int>> phis(num_X_indices);
    for(int i = 0; i < num_X_indices; i++) {
        phis[i] = i;
    }
    return BitvectorPredicateAbstraction(phis);
}

BitvectorPredicateAbstraction BitvectorPredicateDomain::meetXModelsPhi(const BitvectorPredicateAbstraction &element) const {
    if(element.isBottomElement()) {
        return element;
    }
    std::vector<std::optional<int>> phis;
    for(std::vector<std::optional<int>>::const_iterator i = element.predicates.begin(); i != element.predicates.end(); i++) {
        // The grammar should enforce that we always have i->has_value()
        int index = i->value();
        if(x[index]) {
            phis.push_back(*i);
        }
    }
    return BitvectorPredicateAbstraction(phis);
}

BitvectorPredicateAbstraction BitvectorPredicateDomain::meetXNotModelsPhi(const BitvectorPredicateAbstraction &element) const {
    if(element.isBottomElement()) {
        return element;
    }
    std::vector<std::optional<int>> phis;
    for(std::vector<std::optional<int>>::const_iterator i = element.predicates.begin(); i != element.predicates.end(); i++) {
        // The grammar should enforce that we always have i->has_value()
        int index = i->value();
        if(!x[index]) { // This is the only line that differs from meetXModelsPhi
            phis.push_back(*i);
        }
    }
    return BitvectorPredicateAbstraction(phis);
}

BitvectorPredicateAbstraction BitvectorPredicateDomain::binary_join(const BitvectorPredicateAbstraction &e1, const BitvectorPredicateAbstraction &e2) const {
    if(e1.isBottomElement()) {
        return e2;
    } else if(e2.isBottomElement()) {
        return e1;
    }
    std::vector<std::optional<int>> phis = e1.predicates;
    for(std::vector<std::optional<int>>::const_iterator i = e2.predicates.begin(); i != e2.predicates.end(); i++) {
        bool contains_flag = false;
        for(std::vector<std::optional<int>>::const_iterator j = phis.begin(); j != phis.end(); j++) {
            if(*i == *j) {
                contains_flag = true;
                break;
            }
        }
        if(!contains_flag) {
            phis.push_back(*i);
        }
    }
    return BitvectorPredicateAbstraction(phis);
}

Interval<double> SingleIntervalDomain::binary_join(const Interval<double> &e1, const Interval<double> &e2) const {
    return Interval<double>::join(e1, e2);
}

/**
 * Actual box domain instantiation member functions
 */

bool couldBeEmpty(const BooleanDropoutSet::DropoutCounts &counts) {
    return counts.pos + counts.neg <= counts.num_dropout;
}

bool mustBeEmpty(const BooleanDropoutSet::DropoutCounts &counts) {
    return counts.pos + counts.neg == 0;
}

BitvectorPredicateAbstraction SimplestBoxDomain::bestSplit(const BooleanDropoutSet &training_set_abstraction) const {
    std::vector<std::pair<BooleanDropoutSet::DropoutCounts, BooleanDropoutSet::DropoutCounts>> counts;
    std::vector<std::optional<int>> forall_nontrivial, exists_nontrivial;

    for(int i = 0; i < num_X_indices; i++) {
        counts.push_back(training_set_abstraction.splitCounts(i));
        if(!couldBeEmpty(counts[i].first) && !couldBeEmpty(counts[i].second)) {
            forall_nontrivial.push_back(i);
        }
        if(!mustBeEmpty(counts[i].first) && !mustBeEmpty(counts[i].second)) {
            exists_nontrivial.push_back(i);
        }
    }

    if(forall_nontrivial.size() == 0) {
        exists_nontrivial.push_back({}); // The none option type value
        return BitvectorPredicateAbstraction(exists_nontrivial);
    }

    // Compute and store all of the predicates' scores
    std::map<const int, Interval<double>> scores;
    for(std::vector<std::optional<int>>::const_iterator i = exists_nontrivial.begin(); i != exists_nontrivial.end(); i++) {
        int index = i->value(); // Prior for loop guarantees i->has_value()
        Interval<double> temp = jointImpurity(std::make_pair(counts[index].first.pos, counts[index].first.neg),
                                              counts[index].first.num_dropout,
                                              std::make_pair(counts[index].second.pos, counts[index].second.neg),
                                              counts[index].second.num_dropout);
        scores.insert(std::make_pair(index, temp));
    }

    // Find the threshold using only predicates from forall_nontrivial
    double min_upper_bound; // Always gets initialized unless forall_nontrivial.size() == 0,
                            // and we prior have an if statement to check that
    for(std::vector<std::optional<int>>::const_iterator i = forall_nontrivial.begin(); i != forall_nontrivial.end(); i++) {
        int index = i->value(); // Again, guaranteed i->has_value()
        if(i == forall_nontrivial.begin() || min_upper_bound > scores[index].get_upper_bound()) {
            min_upper_bound = scores[index].get_upper_bound();
        }
    }

    // Return any predicates in exists_nontrivial whose score could beat the threshold
    std::vector<std::optional<int>> ret;
    for(std::vector<std::optional<int>>::const_iterator i = exists_nontrivial.begin(); i != exists_nontrivial.end(); i++) {
        int index = i->value(); // Again again, guaranteed i->has_value()
        if(scores[index].get_lower_bound() <= min_upper_bound) {
            ret.push_back(*i);
        }
    }
    return BitvectorPredicateAbstraction(ret);
}

BooleanDropoutSet SimplestBoxDomain::filter(const BooleanDropoutSet &training_set_abstraction, const BitvectorPredicateAbstraction &predicate_abstraction) const {
    std::vector<BooleanDropoutSet> joins;
    for(std::vector<std::optional<int>>::const_iterator i = predicate_abstraction.predicates.begin(); i != predicate_abstraction.predicates.end(); i++) {
        // The grammar should enforce that each i->has_value()
        BooleanDropoutSet temp = training_set_abstraction.filter(i->value(), true);
        joins.push_back(temp);
    }
    return training_set_domain.join(joins);
}

BooleanDropoutSet SimplestBoxDomain::filterNegated(const BooleanDropoutSet &training_set_abstraction, const BitvectorPredicateAbstraction &predicate_abstraction) const {
    // XXX copy and pasted previous method with one change; refactor with common method
    std::vector<BooleanDropoutSet> joins;
    for(std::vector<std::optional<int>>::const_iterator i = predicate_abstraction.predicates.begin(); i != predicate_abstraction.predicates.end(); i++) {
        // The grammar should enforce that each i->has_value()
        BooleanDropoutSet temp = training_set_abstraction.filter(i->value(), false);
        joins.push_back(temp);
    }
    return training_set_domain.join(joins);
}

Interval<double> SimplestBoxDomain::summary(const BooleanDropoutSet &training_set_abstraction) const {
    std::pair<int, int> counts = training_set_abstraction.baseCounts();
    return estimateBernoulli(counts.first, counts.second, training_set_abstraction.num_dropout);
}
