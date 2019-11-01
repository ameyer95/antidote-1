#ifndef CONCRETESEMANTICS_H
#define CONCRETESEMANTICS_H

#include "ASTNode.h"
#include "CategoricalDistribution.hpp"
#include "ConcreteTrainingReferences.h"
#include "Feature.hpp"
#include "Predicate.hpp"
#include <optional>
#include <utility>
#include <vector>


// A struct for the program state
struct ConcreteState {
    ConcreteTrainingReferences training_references;
    std::optional<Predicate> phi;
    CategoricalDistribution<double> posterior;
};


// The actual semantics class (and a struct for the program state)
class ConcreteSemantics : public ASTVisitor {
private:
    FeatureVector test_input;
    ConcreteState current_state;
    CategoricalDistribution<double> return_value; // Largely a proxy for current_state.posterior

public:
    ConcreteSemantics() {}
    // This class doesn't do any dynamic allocation and accordingly does not handle any deallocation

    CategoricalDistribution<double> execute(const FeatureVector &test_input, const DataSet *training_set, const ProgramNode *program);

    void visit(const ProgramNode &node);
    void visit(const SequenceNode &node);
    void visit(const ITEImpurityNode &node);
    void visit(const ITENoPhiNode &node);
    void visit(const BestSplitNode &node);
    void visit(const SummaryNode &node);
    void visit(const UsePhiSequenceNode &node);
    void visit(const ITEModelsNode &node);
    void visit(const FilterNode &node);
    void visit(const ReturnNode &node);
};


#endif
