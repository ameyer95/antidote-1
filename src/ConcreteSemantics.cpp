#include "ConcreteSemantics.h"
#include "ASTNode.h"
#include "DataReferences.h"
#include "information_math.h"
#include <utility>
#include <vector>
using namespace std;

ConcreteSemantics::ConcreteSemantics() {
    //TODO
}

double ConcreteSemantics::execute(const Input test_input, BooleanDataSet *training_set, const vector<BitVectorPredicate> *predicates, const ProgramNode *program) {
    this->test_input = test_input;
    this->training_set = training_set;
    this->predicates = predicates;
    phi = NULL;
    program->accept(*this);
    return return_value;
}

void ConcreteSemantics::visit(const ProgramNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

void ConcreteSemantics::visit(const SequenceNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

void ConcreteSemantics::visit(const ITEImpurityNode &node) {
    if(training_set->isPure()) {
        node.get_left_child()->accept(*this);
    } else {
        node.get_right_child()->accept(*this);
    }
}

void ConcreteSemantics::visit(const ITENoPhiNode &node) {
    if(phi == NULL) {
        node.get_left_child()->accept(*this);
    } else {
        node.get_right_child()->accept(*this);
    }
}

void ConcreteSemantics::visit(const BestSplitNode &node) {
    phi = training_set->bestSplit(predicates);
}

void ConcreteSemantics::visit(const SummaryNode &node) {
    posterior = training_set->summary();
}

void ConcreteSemantics::visit(const UsePhiSequenceNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

void ConcreteSemantics::visit(const ITEModelsNode &node) {
    if(phi->evaluate(test_input)) {
        node.get_left_child()->accept(*this);
    } else {
        node.get_right_child()->accept(*this);
    }
}

void ConcreteSemantics::visit(const FilterNode &node) {
    training_set->filter(*phi, node.get_mode());
}

void ConcreteSemantics::visit(const ReturnNode &node) {
    return_value = posterior;
}

int BooleanDataSet::countOnes() {
    int count = 0;
    for(unsigned int i = 0; i < data->size(); i++) {
        if(classificationBit(i)) {
            count++;
        }
    }
    return count;
}

pair<BinarySamples, BinarySamples> BooleanDataSet::splitCounts(const BitVectorPredicate *phi) {
    pair<BinarySamples, BinarySamples> ret({0, 0}, {0, 0});
    BinarySamples *b_ptr;
    int *count_ptr;
    for(unsigned int i = 0; i < data->size(); i++) {
        // Convention here is that things that do satisfy the predicate have "index" 1; those that don't have 0
        b_ptr = phi->evaluate(getRow(i).first) ? &(ret.second) : &(ret.first);
        count_ptr = classificationBit(i) ? &(b_ptr->num_ones) : &(b_ptr->num_zeros);
        *count_ptr += 1;
    }
    return ret;
}

bool BooleanDataSet::isPure() {
    int count = countOnes();
    return count == 0 || count == data->size();
};

void BooleanDataSet::filter(const BitVectorPredicate &phi, bool mode) {
    bool remove, result;
    // XXX this iterative removal is potentially inefficient; consider a linked list
    for(unsigned int i = 0; i < data->size(); i++) {
        result = phi.evaluate((*data)[i].first);
        remove = (mode != result);
        if(remove) {
            data->remove(i);
            i--;
        }
    }
}

double BooleanDataSet::summary() {
    BinarySamples counts;
    counts.num_ones = countOnes();
    counts.num_zeros = data->size() - counts.num_ones;
    return estimateBernoulli(counts);
}

bool emptyCount(const BinarySamples &counts) {
    return counts.num_zeros == 0 && counts.num_ones == 0;
}

const BitVectorPredicate* BooleanDataSet::bestSplit(const vector<BitVectorPredicate> *predicates) {
    double best_score, current_score;
    const BitVectorPredicate *best_predicate = NULL;
    pair<BinarySamples, BinarySamples> counts;
    for(vector<BitVectorPredicate>::const_iterator i = predicates->begin(); i != predicates->end(); i++) {
        counts = splitCounts(&(*i));
        if(!emptyCount(counts.first) && !emptyCount(counts.second)) {
            current_score = jointImpurity(counts.first, counts.second);
            if(best_predicate == NULL || current_score < best_score) { // Minimize joint impurity
                best_score = current_score;
                best_predicate = &(*i); // Need the address of the iterator's current element
            }
        }
    }
    return best_predicate;
}
