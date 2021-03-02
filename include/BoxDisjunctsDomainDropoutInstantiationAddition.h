#ifndef BOXDISJUNCTSDOMAINDROPOUTINSTANTIATIONADDITION_H
#define BOXDISJUNCTSDOMAINDROPOUTINSTANTIATIONADDITION_H

#include "BoxDisjunctsDomainTemplateAddition.hpp"
#include "BoxStateDomainDropoutInstantiationAddition.h"
#include <utility>
#include <vector>


class BoxDisjunctsDomainDropoutInstantiationAddition : public BoxDisjunctsDomainTemplateAddition<TrainingReferencesWithDropoutAddition, PredicateAbstractionAddition, PosteriorDistributionAbstractionAddition> {
public:
    using BoxDisjunctsDomainTemplateAddition::BoxDisjunctsDomainTemplateAddition; // Inherit constructor

    std::vector<std::pair<TrainingReferencesWithDropoutAddition, PredicateAbstractionAddition>> filter(const TrainingReferencesWithDropoutAddition &training_set_abstraction, const PredicateAbstractionAddition &predicate_abstraction) const;
    std::vector<std::pair<TrainingReferencesWithDropoutAddition, PredicateAbstractionAddition>> filterNegated(const TrainingReferencesWithDropoutAddition &training_set_abstraction, const PredicateAbstractionAddition &predicate_abstraction) const;
};


#endif
