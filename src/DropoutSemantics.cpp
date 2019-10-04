#include "DropoutSemantics.h"
#include "ASTNode.h"
#include "ConcreteSemantics.h"
#include "data_common.h"
#include "information_math.h"
#include "Interval.h"
#include <algorithm> // For std::max etc
#include <map>
#include <utility>
#include <vector>

/**
 * DropoutSet member functions
 */

Interval<int> DropoutSet::countOnes() {
    int count = 0;
    for(unsigned int i = 0; i < data.size(); i++) {
        if(classificationBit(i)) {
            count++;
        }
    }
    return Interval<int>(std::max(count - num_dropout, 0), count);
}

std::pair<DropoutCounts, DropoutCounts> DropoutSet::splitCounts(const BitVectorPredicate &phi) {
    std::pair<DropoutCounts, DropoutCounts> ret({{0, 0}, 0}, {{0, 0}, 0});
    DropoutCounts *d_ptr;
    int *count_ptr;
    for(unsigned int i = 0; i < data.size(); i++) {
        d_ptr = phi.evaluate(getRow(i).first) ? &(ret.second) : &(ret.first);
        count_ptr = classificationBit(i) ? &(d_ptr->bsamples.num_ones) : &(d_ptr->bsamples.num_zeros);
        *count_ptr += 1;
    }
    std::vector<DropoutCounts*> iters = {&(ret.first), &(ret.second)};
    for(std::vector<DropoutCounts*>::iterator i = iters.begin(); i != iters.end(); i++) {
        (*i)->num_dropout = std::min(num_dropout, (*i)->bsamples.num_ones + (*i)->bsamples.num_zeros);
    }
    return ret;
}

DropoutSet* DropoutSet::pureSets(bool classification) {
    DataReferences<DataRow> datacopy = data;
    int num_removed = 0;
    for(unsigned int i = 0; i < datacopy.size(); i++) {
        if(datacopy[i].second != classification) {
            datacopy.remove(i);
            num_removed++;
            i--;
        }
    }
    if(num_removed <= num_dropout) {
        return new DropoutSet(datacopy, num_dropout - num_removed);
    } else {
        return NULL;
    }
};

bool couldBeEmpty(const DropoutCounts &counts) {
    return counts.bsamples.num_zeros + counts.bsamples.num_ones <= counts.num_dropout;
}

bool mustBeEmpty(const DropoutCounts &counts) {
    return counts.bsamples.num_zeros + counts.bsamples.num_ones == 0;
}

PredicatePointers DropoutSet::bestSplit(const PredicateSet *predicates) {
    std::map<const BitVectorPredicate*, std::pair<DropoutCounts, DropoutCounts>> counts;
    PredicatePointers forall_nontrivial, exists_nontrivial;

    for(PredicateSet::const_iterator i = predicates->begin(); i != predicates->end(); i++) {
        counts.insert(std::make_pair(&(*i), splitCounts(*i)));
        if(!couldBeEmpty(counts[&(*i)].first) && !couldBeEmpty(counts[&(*i)].second)) {
            forall_nontrivial.push_back(&(*i));
        }
        if(!mustBeEmpty(counts[&(*i)].first) && !mustBeEmpty(counts[&(*i)].second)) {
            exists_nontrivial.push_back(&(*i));
        }
    }

    if(forall_nontrivial.size() == 0) {
        exists_nontrivial.push_back(NULL);
        return exists_nontrivial;
    }

    // Compute and store all of the predicates' scores
    std::map<const BitVectorPredicate*, Interval<double>> scores;
    for(PredicatePointers::const_iterator i = exists_nontrivial.begin(); i != exists_nontrivial.end(); i++) {
        Interval<double> temp = jointImpurity(counts[*i].first.bsamples,
                                              counts[*i].first.num_dropout,
                                              counts[*i].second.bsamples,
                                              counts[*i].second.num_dropout);
        scores.insert(std::make_pair(*i, temp));
    }

    // Find the threshold using only predicates from forall_nontrivial
    double min_upper_bound; // Always gets initialized unless forall_nontrivial.size() == 0,
                            // and we prior have an if statement to check that
    for(PredicatePointers::const_iterator i = forall_nontrivial.begin(); i != forall_nontrivial.end(); i++) {
        if(i == forall_nontrivial.begin() || min_upper_bound > scores[*i].get_upper_bound()) {
            min_upper_bound = scores[*i].get_upper_bound();
        }
    }

    // Return any predicates in exists_nontrivial whose score could beat the threshold
    PredicatePointers ret;
    for(PredicatePointers::iterator i = exists_nontrivial.begin(); i != exists_nontrivial.end(); i++) {
        if(scores[*i].get_lower_bound() <= min_upper_bound) {
            ret.push_back(*i);
        }
    }

    return ret;
}

void DropoutSet::filter(const BitVectorPredicate &phi, bool mode) {
    bool remove, result;
    for(unsigned int i = 0; i < data.size(); i++) {
        result = phi.evaluate(data[i].first);
        remove = (mode != result);
        if(remove) {
            data.remove(i);
            i--;
        }
    }
    if(num_dropout > data.size()) {
        num_dropout = data.size();
    }
}

Interval<double> DropoutSet::summary() {
    BinarySamples counts;
    counts.num_ones = countOnes().get_upper_bound();
    counts.num_zeros = data.size() - counts.num_ones;
    return estimateBernoulli(counts, num_dropout);
}

// In this (and other places), we're assuming an invariant that data.size() >= num_dropout
Interval<int> DropoutSet::size() {
    return Interval<int>(data.size() - num_dropout, data.size());
}

DropoutSet DropoutSet::join(const DropoutSet &e1, const DropoutSet &e2) {
    if(e1.isBottom()) {
        return e2;
    } else if(e2.isBottom()) {
        return e1;
    }
    DataReferences<DataRow> d = DataReferences<DataRow>::set_union(e1.data, e2.data);
    int n1 = d.size() - e2.data.size() + e2.num_dropout; // Note |(T1 U T2) \ T1| = |T2 \ T1|
    int n2 = d.size() - e1.data.size() + e1.num_dropout;
    return DropoutSet(d, std::max(n1, n2));
}

DropoutSet DropoutSet::join(const vector<DropoutSet> &elements) {
    DropoutSet ret;
    for(vector<DropoutSet>::const_iterator i = elements.begin(); i != elements.end(); i++) {
        ret = DropoutSet::join(ret, *i);
    }
    return ret;
}

/**
 * AbstractState member functions
 */

AbstractState AbstractState::join(const AbstractState &e1, const AbstractState &e2) {
    if(e1.bot_flag) {
        return e2;
    } else if(e2.bot_flag) {
        return e1;
    }
    AbstractState ret;
    ret.training_set = DropoutSet::join(e1.training_set, e2.training_set);
    ret.phis = e1.phis;
    for(PredicatePointers::const_iterator i = e2.phis.begin(); i != e2.phis.end(); i++) {
        bool contains_flag = false;
        for(PredicatePointers::const_iterator j = ret.phis.begin(); j != ret.phis.end(); j++) {
            if(*i == *j) {
                contains_flag = true;
                break;
            }
        }
        if(!contains_flag) {
            ret.phis.push_back(*i);
        }
    }
    ret.posterior = Interval<double>::join(e1.posterior, e2.posterior);
    ret.bot_flag = false;
    return ret;
}

AbstractState AbstractState::join(const vector<AbstractState> &elements) {
    AbstractState ret;
    for(vector<AbstractState>::const_iterator i = elements.begin(); i != elements.end(); i++) {
        ret = AbstractState::join(ret, *i);
    }
    return ret;
}

/**
 * DropoutSemantics member functions
 */

DropoutSemantics::DropoutSemantics() {
    // TODO
}

Interval<double> DropoutSemantics::execute(const Input test_input, DropoutSet *training_set, const PredicateSet *predicates, const ProgramNode *program) {
    this->test_input = test_input;
    current_state.training_set = *training_set;
    current_state.phis.clear();
    current_state.bot_flag = false;
    this->predicates = predicates;
    program->accept(*this);
    return return_value;
}

void DropoutSemantics::visit(const ProgramNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

void DropoutSemantics::visit(const SequenceNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

void DropoutSemantics::visit(const ITEImpurityNode &node) {
    DropoutSet *pure0, *pure1;
    std::vector<AbstractState> joins;
    AbstractState backup;

    pure0 = current_state.training_set.pureSets(false);
    pure1 = current_state.training_set.pureSets(true);

    if(pure0 != NULL) {
        backup = current_state;
        current_state.training_set = *pure0;
        node.get_left_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }
    
    if(pure1 != NULL) {
        backup = current_state;
        current_state.training_set = *pure1;
        node.get_left_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    node.get_right_child()->accept(*this);
    joins.push_back(current_state);

    current_state = AbstractState::join(joins);

    delete pure0;
    delete pure1;
}

void DropoutSemantics::visit(const ITENoPhiNode &node) {
    std::vector<AbstractState> joins;
    PredicatePointers::iterator i;
    bool contains_bot = false;
    AbstractState backup;

    for(i = current_state.phis.begin(); i != current_state.phis.end(); i++) {
        if((*i) == NULL) {
            contains_bot = true;
            break;
        }
    }

    if(contains_bot) {
        current_state.phis.erase(i);
    }

    backup = current_state;
    node.get_right_child()->accept(*this);
    joins.push_back(current_state);
    current_state = backup;

    if(contains_bot) {
        backup = current_state;
        current_state.phis = PredicatePointers({NULL});
        node.get_left_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    current_state = AbstractState::join(joins);
}

void DropoutSemantics::visit(const BestSplitNode &node) {
    current_state.phis = current_state.training_set.bestSplit(predicates);
}

void DropoutSemantics::visit(const SummaryNode &node) {
    current_state.posterior = current_state.training_set.summary();
}

void DropoutSemantics::visit(const UsePhiSequenceNode &node) {
    node.get_left_child()->accept(*this);
    node.get_right_child()->accept(*this);
}

void DropoutSemantics::visit(const ITEModelsNode &node) {
    std::vector<AbstractState> joins;
    AbstractState backup;
    PredicatePointers pos, neg;

    for(PredicatePointers::iterator i = current_state.phis.begin(); i != current_state.phis.end(); i++) {
        if((*i)->evaluate(test_input)) {
            pos.push_back(*i);
        } else {
            neg.push_back(*i);
        }
    }

    if(pos.size() > 0) {
        backup = current_state;
        current_state.phis = pos;
        node.get_left_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    if(neg.size() > 0) {
        backup = current_state;
        current_state.phis = neg;
        node.get_right_child()->accept(*this);
        joins.push_back(current_state);
        current_state = backup;
    }

    current_state = AbstractState::join(joins);
}

void DropoutSemantics::visit(const FilterNode &node) {
    std::vector<DropoutSet> joins;
    DropoutSet temp;
    for(PredicatePointers::iterator i = current_state.phis.begin(); i != current_state.phis.end(); i++) {
        temp = current_state.training_set;
        temp.filter(**i, node.get_mode());
        joins.push_back(temp);
    }
    current_state.training_set = DropoutSet::join(joins);
}

void DropoutSemantics::visit(const ReturnNode &node) {
    // Grammar enforces only a single return statement at the end, so we need not join
    return_value = current_state.posterior;
}
