#ifndef ABSTRACTSEMANTICSINSTANTIATIONS_HPP
#define ABSTRACTSEMANTICSINSTANTIATIONS_HPP

/**
 * This file provides the instantations of the abstract semantics template class
 * so that a single source file can handle compiling the object code.
 * Files wanting to use abstract semantics should include THIS file.
 */

#include "AbstractSemanticsTemplate.h"
#include "BoxStateDomainDropoutInstantiation.h"
#include "BoxStateDomainDropoutInstantiationLabels.h"
#include "BoxDisjunctsDomainDropoutInstantiation.h"
#include "BoxDisjunctsDomainDropoutInstantiationLabels.h"


// Forward-declare the types to get code generation
template class AbstractSemanticsTemplate<BoxDropoutDomain::AbstractionType>;
template class AbstractSemanticsTemplate<BoxDropoutDomainLabels::AbstractionType>;
template class AbstractSemanticsTemplate<BoxDisjunctsDomainDropoutInstantiation::AbstractionType>;
template class AbstractSemanticsTemplate<BoxDisjunctsDomainDropoutInstantiationLabels::AbstractionType>;

// Give them nicer names
typedef AbstractSemanticsTemplate<BoxDropoutDomain::AbstractionType> BoxDropoutSemantics;
typedef AbstractSemanticsTemplate<BoxDropoutDomainLabels::AbstractionType> BoxDropoutSemanticsLabels;
typedef AbstractSemanticsTemplate<BoxDisjunctsDomainDropoutInstantiation::AbstractionType> BoxDisjunctsDropoutSemantics;
typedef AbstractSemanticsTemplate<BoxDisjunctsDomainDropoutInstantiationLabels::AbstractionType> BoxDisjunctsDropoutSemanticsLabels;

#endif
