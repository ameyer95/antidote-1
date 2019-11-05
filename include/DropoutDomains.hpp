#ifndef DROPOUTDOMAINS_HPP
#define DROPOUTDOMAINS_HPP

#include "BoxBoundedDisjunctsDomainDropoutInstantiation.h"
#include "BoxDisjunctsDomainDropoutInstantiation.h"
#include "BoxStateDomainDropoutInstantiation.h"

/**
 * This file maintains construction of the various Box-based abstract domains
 * for the DSL program state.
 * It's purely for convenience,
 * as the disjunctive domain requires having constructed the single domain, etc.
 */


class DropoutDomains {
public:
    TrainingSetDropoutDomain T_domain;
    PredicateSetDomain Phi_domain;
    PosteriorDistributionIntervalDomain D_domain;
    BoxDropoutDomain box_domain;
    BoxDisjunctsDomainDropoutInstantiation disjuncts_domain;
    BoxBoundedDisjunctsDomainDropoutInstantiation bounded_disjuncts_domain;

    DropoutDomains() : box_domain(&T_domain, &Phi_domain, &D_domain),
                       disjuncts_domain(&box_domain),
                       bounded_disjuncts_domain(&disjuncts_domain) {}
};


#endif
