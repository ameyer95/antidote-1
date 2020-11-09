#ifndef DROPOUTDOMAINSLABELS_HPP
#define DROPOUTDOMAINSLABELS_HPP

#include "BoxBoundedDisjunctsDomainDropoutInstantiationLabels.h"
#include "BoxDisjunctsDomainDropoutInstantiationLabels.h"
#include "BoxStateDomainDropoutInstantiationLabels.h"

/**
 * This file maintains construction of the various Box-based abstract domains
 * for the DSL program state.
 * It's purely for convenience,
 * as the disjunctive domain requires having constructed the single domain, etc.
 */


class DropoutDomainsLabels {
public:
    TrainingSetDropoutDomainLabels T_domain;
    PredicateSetDomainLabels Phi_domain;
    PosteriorDistributionIntervalDomainLabels D_domain;
    BoxDropoutDomainLabels box_domain;
    BoxDisjunctsDomainDropoutInstantiationLabels disjuncts_domain;
    BoxBoundedDisjunctsDomainDropoutInstantiationLabels bounded_disjuncts_domain;

    DropoutDomainsLabels() : box_domain(&T_domain, &Phi_domain, &D_domain),
                             disjuncts_domain(&box_domain),
                             bounded_disjuncts_domain(&disjuncts_domain) {}
};


#endif
