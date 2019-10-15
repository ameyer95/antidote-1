#include "SimplestBoxDisjunctsInstantiation.h"
#include "SimplestBoxInstantiation.h"
#include <algorithm> // for std::max/min
#include <utility>
#include <vector>

/**
 * Unbounded domain member functions
 */

std::vector<std::pair<BooleanDropoutSet, BitvectorPredicateAbstraction>> SimplestBoxDisjunctsDomain::filter(const BooleanDropoutSet &training_set_abstraction, const BitvectorPredicateAbstraction &predicate_abstraction) const {
    // This is a simple adaptation of the SimplestBoxDomain::filter.
    // The main difference is that we don't actually perform a join over the different predicate outcome possibilities
    std::vector<std::pair<BooleanDropoutSet, BitvectorPredicateAbstraction>> joins;
    for(std::vector<std::optional<int>>::const_iterator i = predicate_abstraction.predicates.begin(); i != predicate_abstraction.predicates.end(); i++) {
        // The grammar should enforce that each i->has_value()
        BooleanDropoutSet temp = training_set_abstraction.filter(i->value(), true);
        joins.push_back(std::make_pair(temp, BitvectorPredicateAbstraction({*i})));
    }
    return joins;
}

std::vector<std::pair<BooleanDropoutSet, BitvectorPredicateAbstraction>> SimplestBoxDisjunctsDomain::filterNegated(const BooleanDropoutSet &training_set_abstraction, const BitvectorPredicateAbstraction &predicate_abstraction) const {
    // XXX mostly a copy-paste of previous method
    std::vector<std::pair<BooleanDropoutSet, BitvectorPredicateAbstraction>> joins;
    for(std::vector<std::optional<int>>::const_iterator i = predicate_abstraction.predicates.begin(); i != predicate_abstraction.predicates.end(); i++) {
        BooleanDropoutSet temp = training_set_abstraction.filter(i->value(), false);
        joins.push_back(std::make_pair(temp, BitvectorPredicateAbstraction({*i})));
    }
    return joins;
}

/**
 * Bounded domain member functions
 */

SimplestBoxBoundedDisjunctsDomain::SimplestBoxBoundedDisjunctsDomain(const SimplestBoxDomain *box_domain, unsigned int max_num_disjuncts, MergeMode merge_mode) {
    disjuncts_domain = new SimplestBoxDisjunctsDomain(box_domain);
    this->max_num_disjuncts = max_num_disjuncts;
    this->merge_mode = merge_mode;
}

SimplestBoxBoundedDisjunctsDomain::~SimplestBoxBoundedDisjunctsDomain() {
    delete disjuncts_domain;
}

double SimplestBoxBoundedDisjunctsDomain::joinPrecisionLoss(const SimplestBoxAbstraction &e1, const SimplestBoxAbstraction &e2) const {
    SimplestBoxAbstraction ej = disjuncts_domain->box_domain->binary_join(e1, e2);
    // We'll approximate the raw number of erroneous concrete training sets introduced by the join
    // by looking at the increase in the num_dropout (relative to the size of the resultant base training set)
    int dropout_increase = ej.training_set_abstraction.num_dropout - std::min(e1.training_set_abstraction.num_dropout, e2.training_set_abstraction.num_dropout);
    return (double)dropout_increase / ej.training_set_abstraction.training_set.size();
}
