#include "BoxBoundedDisjunctsDomainDropoutInstantiation.h"
#include <algorithm> // for std::min

double BoxBoundedDisjunctsDomainDropoutInstantiation::joinPrecisionLoss(const Types::Single &e1, const Types::Single &e2) const {
    Types::Single ej = disjuncts_domain->box_domain->binary_join(e1, e2);
    // We'll approximate the raw number of erroneous concrete training sets introduced by the join
    // by looking at the increase in the num_dropout (relative to the size of the resultant base training set)
    int dropout_increase = ej.training_set_abstraction.num_dropout - std::min(e1.training_set_abstraction.num_dropout, e2.training_set_abstraction.num_dropout);
    return (double)dropout_increase / ej.training_set_abstraction.training_references.size();
}
