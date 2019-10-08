#ifndef SIMPLESTBOXDISJUNCTSINSTANTIATION_H
#define SIMPLESTBOXDISJUNCTSINSTANTIATION_H

/**
 * In this file, we'll handle the simplest instantiations for both the
 * (unbounded) disjuncts of box abstractions and the
 * bounded disjuncts of box abstractions.
 */

#include "BoxBoundedDisjunctsDomain.h"
#include "BoxDisjunctsDomain.h"
#include "SimplestBoxInstantiation.h"
#include <utility>
#include <vector>

typedef BoxDisjunctsStateAbstraction<SimplestBoxAbstraction> SimplestBoxDisjunctsAbstraction;


class SimplestBoxDisjunctsDomain : public BoxDisjunctsDomain<SimplestBoxDisjunctsAbstraction, SimplestBoxDomain, SimplestBoxAbstraction, BooleanDropoutSet, BitvectorPredicateAbstraction> {
public:
    SimplestBoxDisjunctsDomain(const SimplestBoxDomain *box_domain) { this->box_domain = box_domain; }

    std::vector<std::pair<BooleanDropoutSet, BitvectorPredicateAbstraction>> filter(const BooleanDropoutSet &training_set_abstraction, const BitvectorPredicateAbstraction &predicate_abstraction) const;
    std::vector<std::pair<BooleanDropoutSet, BitvectorPredicateAbstraction>> filterNegated(const BooleanDropoutSet &training_set_abstraction, const BitvectorPredicateAbstraction &predicate_abstraction) const;
};


class SimplestBoxBoundedDisjunctsDomain : public BoxBoundedDisjunctsDomain<SimplestBoxDisjunctsDomain, SimplestBoxDisjunctsAbstraction, SimplestBoxAbstraction, double> {
public:
    SimplestBoxBoundedDisjunctsDomain(const SimplestBoxDomain *box_domain, unsigned int max_num_disjuncts);
    ~SimplestBoxBoundedDisjunctsDomain();

    double joinPrecisionLoss(const SimplestBoxAbstraction &e1, const SimplestBoxAbstraction &e2) const;
};


#endif
