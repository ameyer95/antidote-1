#include "ConcreteSemantics.h"
#include "ASTNode.h"
#include "concrete_common.h"
#include "data_common.h"
#include "information_math.h"
#include <math.h> // for isnan
#include <utility>
#include <vector>
using namespace std;

ConcreteSemantics::ConcreteSemantics() {
    //TODO
}

double ConcreteSemantics::execute(const Input test_input, BooleanDataSet *training_set, const vector<BitVectorPredicate> *predicates, const ASTNode *program) {
    this->test_input = test_input;
    this->training_set = training_set;
    this->predicates = predicates;
    halt = false;
    program->accept(*this);
    return return_value;
}

void ConcreteSemantics::visit(const SequenceNode &node) {
    if(halt) return;
    for(int i = 0; i < node.get_num_children(); i++) {
        node.get_child(i)->accept(*this);
    }
}

void ConcreteSemantics::visit(const ITEImpurityNode &node) {
    if(halt) return;
    if(training_set->isPure()) {
        node.get_then_child()->accept(*this);
    } else {
        node.get_else_child()->accept(*this);
    }
}

void ConcreteSemantics::visit(const ITEModelsNode &node) {
    if(halt) return;
    if(phi->evaluate(test_input)) {
        node.get_then_child()->accept(*this);
    } else {
        node.get_else_child()->accept(*this);
    }
}

void ConcreteSemantics::visit(const BestSplitNode &node) {
    if(halt) return;
    phi = training_set->bestSplit(predicates);
}

void ConcreteSemantics::visit(const FilterNode &node) {
    if(halt) return;
    training_set->filter(*phi, node.get_mode());
}

void ConcreteSemantics::visit(const SummaryNode &node) {
    if(halt) return;
    posterior = training_set->summary();
}

void ConcreteSemantics::visit(const ReturnNode &node) {
    if(halt) return;
    return_value = posterior;
    halt = true;
}

int BooleanDataSet::countOnes() {
    int count = 0;
    for(int i = 0; i < data->size(); i++) {
        if(classificationBit(i)) {
            count++;
        }
    }
    return count;
}

pair<pair<int, int>, pair<int, int>> BooleanDataSet::splitCounts(const BitVectorPredicate *phi) {
    pair<pair<int, int>, pair<int, int>> ret(make_pair(0, 0), make_pair(0, 0));
    pair<int, int> *pair_ptr;
    int *count_ptr;
    for(int i = 0; i < data->size(); i++) {
        pair_ptr = phi->evaluate(getRow(i).first) ? &(ret.second) : &(ret.first);
        count_ptr = classificationBit(i) ? &(pair_ptr->second) : &(pair_ptr->first);
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
    for(int i = 0; i < data->size(); i++) {
        result = phi.evaluate((*data)[i].first);
        remove = mode ? result : !result;
        if(remove) {
            data->remove(i);
            i--;
        }
    }
}

double BooleanDataSet::summary() {
    return (double)countOnes() / data->size();
}

const BitVectorPredicate* BooleanDataSet::bestSplit(const vector<BitVectorPredicate> *predicates) {
    double best_score, current_score;
    const BitVectorPredicate *best_predicate = NULL;
    pair<pair<int, int>, pair<int, int>> counts;
    for(vector<BitVectorPredicate>::const_iterator i = predicates->begin(); i != predicates->end(); i++) {
        counts = splitCounts(&(*i));
        current_score = informationGain(counts.first, counts.second);
        //XXX the semantics of this conditional,
        // and what to do when all splits are trivial or provide no gain,
        // are corner cases that really need more careful consideration
        if(best_predicate == NULL || isnan(best_score) || best_score < current_score) {
            best_score = current_score;
            best_predicate = &(*i); // Need the address of the iterator's current element
        }
    }
    return best_predicate;
}
