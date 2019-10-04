#include "SimplestBoxDisjunctsInstantiation.h"
#include "SimplestBoxInstantiation.h"
#include <utility>
#include <vector>

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
