#ifndef SIMPLESTBOXDISJUNCTSINSTANTIATION_H
#define SIMPLESTBOXDISJUNCTSINSTANTIATION_H

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


#endif
