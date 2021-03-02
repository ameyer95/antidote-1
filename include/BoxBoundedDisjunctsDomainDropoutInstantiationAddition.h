#ifndef BOXBOUNDEDDISJUNCTSDOMAINDROPOUTINSTANTIATIONADDITION_H
#define BOXBOUNDEDDISJUNCTSDOMAINDROPOUTINSTANTIATIONADDITION_H

#include "BoxBoundedDisjunctsDomainTemplateAddition.hpp"
#include "BoxStateDomainDropoutInstantiationAddition.h"


class BoxBoundedDisjunctsDomainDropoutInstantiationAddition : public BoxBoundedDisjunctsDomainTemplateAddition<TrainingReferencesWithDropoutAddition, PredicateAbstractionAddition, PosteriorDistributionAbstractionAddition, double> {
public:
    using BoxBoundedDisjunctsDomainTemplateAddition::BoxBoundedDisjunctsDomainTemplateAddition; // Inherit constructors
    double joinPrecisionLoss(const Types::Single &e1, const Types::Single &e2) const;
};


#endif
