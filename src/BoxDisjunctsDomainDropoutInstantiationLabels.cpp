#include "BoxDisjunctsDomainDropoutInstantiationLabels.h"
#include <utility>
#include <vector>

std::vector<std::pair<TrainingReferencesWithDropoutLabels, PredicateAbstractionLabels>> BoxDisjunctsDomainDropoutInstantiationLabels::filter(const TrainingReferencesWithDropoutLabels &training_set_abstraction, const PredicateAbstractionLabels &predicate_abstraction) const {
    // This is a simple adaptation of the BoxStateDomainDropoutInstantiation::filter.
    // The main difference is that we don't actually perform a join over the different predicate outcome possibilities
    std::vector<std::pair<TrainingReferencesWithDropoutLabels, PredicateAbstractionLabels>> joins;
    for(auto i = predicate_abstraction.cbegin(); i != predicate_abstraction.cend(); i++) {
        // The grammar should enforce that each i->has_value()
        TrainingReferencesWithDropoutLabels temp = training_set_abstraction.filter(i->value(), true);
        joins.push_back(std::make_pair(temp, PredicateAbstractionLabels({*i})));
    }
    return joins;
}

std::vector<std::pair<TrainingReferencesWithDropoutLabels, PredicateAbstractionLabels>> BoxDisjunctsDomainDropoutInstantiationLabels::filterNegated(const TrainingReferencesWithDropoutLabels &training_set_abstraction, const PredicateAbstractionLabels &predicate_abstraction) const {
    // XXX mostly a copy-paste of previous method
    std::vector<std::pair<TrainingReferencesWithDropoutLabels, PredicateAbstractionLabels>> joins;
    for(auto i = predicate_abstraction.cbegin(); i != predicate_abstraction.cend(); i++) {
        TrainingReferencesWithDropoutLabels temp = training_set_abstraction.filter(i->value(), false);
        joins.push_back(std::make_pair(temp, PredicateAbstractionLabels({*i})));
    }
    return joins;
}
