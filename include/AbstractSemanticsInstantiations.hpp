#ifndef ABSTRACTSEMANTICSINSTANTIATIONS_HPP
#define ABSTRACTSEMANTICSINSTANTIATIONS_HPP

/**
 * This file provides the instantations of the abstract semantics template class
 * so that a single source file can handle compiling the object code.
 * Files wanting to use abstract semantics should include THIS file.
 */

#include "AbstractSemanticsTemplate.h"
#include "BoxStateDomainDropoutInstantiation.h"
#include "BoxDisjunctsDomainDropoutInstantiation.h"

// Forward-declare the types to get code generation
template class AbstractSemanticsTemplate<BoxDropoutDomain::BoxStateAbstractionType>;
template class AbstractSemanticsTemplate<BoxDisjunctsDomainDropoutInstantiation::BoxDisjunctsType>;

// Give them nicer names
typedef AbstractSemanticsTemplate<BoxDropoutDomain::BoxStateAbstractionType> BoxDropoutSemantics;
typedef AbstractSemanticsTemplate<BoxDisjunctsDomainDropoutInstantiation::BoxDisjunctsType> BoxDisjunctsDropoutSemantics;

#endif
