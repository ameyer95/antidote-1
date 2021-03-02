#ifndef DROPOUTDOMAINSADDITION_HPP
#define DROPOUTDOMAINSADDITION_HPP

#include "BoxBoundedDisjunctsDomainDropoutInstantiationAddition.h"
#include "BoxDisjunctsDomainDropoutInstantiationAddition.h"
#include "BoxStateDomainDropoutInstantiationAddition.h"

/**
 * This file maintains construction of the various Box-based abstract domains
 * for the DSL program state.
 * It's purely for convenience,
 * as the disjunctive domain requires having constructed the single domain, etc.
 */


class DropoutDomainsAddition {
public:
    TrainingSetDropoutDomainAddition T_domain;
    PredicateSetDomainAddition Phi_domain;
    PosteriorDistributionIntervalDomainAddition D_domain;
    BoxDropoutDomainAddition box_domain;
    BoxDisjunctsDomainDropoutInstantiationAddition disjuncts_domain;
    BoxBoundedDisjunctsDomainDropoutInstantiationAddition bounded_disjuncts_domain;

    DropoutDomainsAddition() : box_domain(&T_domain, &Phi_domain, &D_domain),
                             disjuncts_domain(&box_domain),
                             bounded_disjuncts_domain(&disjuncts_domain) {}
};


#endif
