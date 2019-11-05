#ifndef BOXDISJUNCTSDOMAINDROPOUTINSTANTIATION_H
#define BOXDISJUNCTSDOMAINDROPOUTINSTANTIATION_H

#include "BoxDisjunctsDomainTemplate.hpp"
#include "BoxStateDomainDropoutInstantiation.h"
#include <utility>
#include <vector>


class BoxDisjunctsDomainDropoutInstantiation : public BoxDisjunctsDomainTemplate<TrainingReferencesWithDropout, PredicateAbstraction, PosteriorDistributionAbstraction> {
public:
    using BoxDisjunctsDomainTemplate::BoxDisjunctsDomainTemplate; // Inherit constructor

    std::vector<std::pair<TrainingReferencesWithDropout, PredicateAbstraction>> filter(const TrainingReferencesWithDropout &training_set_abstraction, const PredicateAbstraction &predicate_abstraction) const;
    std::vector<std::pair<TrainingReferencesWithDropout, PredicateAbstraction>> filterNegated(const TrainingReferencesWithDropout &training_set_abstraction, const PredicateAbstraction &predicate_abstraction) const;
};


#endif
