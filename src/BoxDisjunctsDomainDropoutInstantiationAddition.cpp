#include "BoxDisjunctsDomainDropoutInstantiationAddition.h"
#include <utility>
#include <vector>

std::vector<std::pair<TrainingReferencesWithDropoutAddition, PredicateAbstractionAddition>> BoxDisjunctsDomainDropoutInstantiationAddition::filter(const TrainingReferencesWithDropoutAddition &training_set_abstraction, const PredicateAbstractionAddition &predicate_abstraction) const {
    // This is a simple adaptation of the BoxStateDomainDropoutInstantiation::filter.
    // The main difference is that we don't actually perform a join over the different predicate outcome possibilities
    std::vector<std::pair<TrainingReferencesWithDropoutAddition, PredicateAbstractionAddition>> joins;
    for(auto i = predicate_abstraction.cbegin(); i != predicate_abstraction.cend(); i++) {
        // The grammar should enforce that each i->has_value()
        TrainingReferencesWithDropoutAddition temp = training_set_abstraction.filter(i->value(), true);
        joins.push_back(std::make_pair(temp, PredicateAbstractionAddition({*i})));
    }
    return joins;
}

std::vector<std::pair<TrainingReferencesWithDropoutAddition, PredicateAbstractionAddition>> BoxDisjunctsDomainDropoutInstantiationAddition::filterNegated(const TrainingReferencesWithDropoutAddition &training_set_abstraction, const PredicateAbstractionAddition &predicate_abstraction) const {
    // XXX mostly a copy-paste of previous method
    std::vector<std::pair<TrainingReferencesWithDropoutAddition, PredicateAbstractionAddition>> joins;
    for(auto i = predicate_abstraction.cbegin(); i != predicate_abstraction.cend(); i++) {
        TrainingReferencesWithDropoutAddition temp = training_set_abstraction.filter(i->value(), false);
        joins.push_back(std::make_pair(temp, PredicateAbstractionAddition({*i})));
    }
    return joins;
}
