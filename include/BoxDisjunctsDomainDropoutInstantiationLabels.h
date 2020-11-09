#ifndef BOXDISJUNCTSDOMAINDROPOUTINSTANTIATIONLABELS_H
#define BOXDISJUNCTSDOMAINDROPOUTINSTANTIATIONLABELS_H

#include "BoxDisjunctsDomainTemplateLabels.hpp"
#include "BoxStateDomainDropoutInstantiationLabels.h"
#include <utility>
#include <vector>


class BoxDisjunctsDomainDropoutInstantiationLabels : public BoxDisjunctsDomainTemplateLabels<TrainingReferencesWithDropoutLabels, PredicateAbstractionLabels, PosteriorDistributionAbstractionLabels> {
public:
    using BoxDisjunctsDomainTemplateLabels::BoxDisjunctsDomainTemplateLabels; // Inherit constructor

    std::vector<std::pair<TrainingReferencesWithDropoutLabels, PredicateAbstractionLabels>> filter(const TrainingReferencesWithDropoutLabels &training_set_abstraction, const PredicateAbstractionLabels &predicate_abstraction) const;
    std::vector<std::pair<TrainingReferencesWithDropoutLabels, PredicateAbstractionLabels>> filterNegated(const TrainingReferencesWithDropoutLabels &training_set_abstraction, const PredicateAbstractionLabels &predicate_abstraction) const;
};


#endif
