#ifndef BOXBOUNDEDDISJUNCTSDOMAINDROPOUTINSTANTIATIONLABELS_H
#define BOXBOUNDEDDISJUNCTSDOMAINDROPOUTINSTANTIATIONLABELS_H

#include "BoxBoundedDisjunctsDomainTemplateLabels.hpp"
#include "BoxStateDomainDropoutInstantiationLabels.h"


class BoxBoundedDisjunctsDomainDropoutInstantiationLabels : public BoxBoundedDisjunctsDomainTemplateLabels<TrainingReferencesWithDropoutLabels, PredicateAbstractionLabels, PosteriorDistributionAbstractionLabels, double> {
public:
    using BoxBoundedDisjunctsDomainTemplateLabels::BoxBoundedDisjunctsDomainTemplateLabels; // Inherit constructors
    double joinPrecisionLoss(const Types::Single &e1, const Types::Single &e2) const;
};


#endif
