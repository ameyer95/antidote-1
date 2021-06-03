#include "BoxStateDomainDropoutInstantiation.h"
#include "Feature.hpp"
#include "information_math.h"
#include "Interval.h"
#include <algorithm>
#include <list>
#include <numeric> // for std::accumulate
#include <set>
#include <utility>
#include <vector>
#include <list>
#include <iostream>

/**
 * TrainingReferencesWithDropout members
 */

TrainingReferencesWithDropout::TrainingReferencesWithDropout(DataReferences training_references, int num_dropout, int num_add, std::pair<int, int> add_sens_info, int num_labels_flip, std::pair<int, int> label_sens_info, int num_features_flip, int feature_flip_index, float feature_flip_amt) {
    this->training_references = training_references;
    this->num_dropout = num_dropout;
    this->num_add = num_add;
    this->add_sens_info = add_sens_info;
    this->num_labels_flip = num_labels_flip;
    this->label_sens_info = label_sens_info;
    this->num_features_flip = num_features_flip;
    this->feature_flip_index = feature_flip_index;
    this->feature_flip_amt = feature_flip_amt;
}

std::vector<int> TrainingReferencesWithDropout::baseCounts() const {
    std::vector<int> counts(training_references.getNumCategories(), 0);
    for(unsigned int i = 0; i < training_references.size(); i++) {
        counts[training_references[i].y]++;
    }
    return counts;
}

std::pair<TrainingReferencesWithDropout::DropoutCounts, TrainingReferencesWithDropout::DropoutCounts> TrainingReferencesWithDropout::splitCounts(const SymbolicPredicate &phi) const {
    // If we are in this case, the feature type is boolean, therefore, we don't have to worry about feature poisoning uncertainty
    std::pair<DropoutCounts, DropoutCounts> ret;
    ret.first.counts = std::vector<int>(training_references.getNumCategories(), 0);
    ret.second.counts = std::vector<int>(training_references.getNumCategories(), 0);
    DropoutCounts *d_ptr;
    int maybe_count = 0;
    for(unsigned int i = 0; i < training_references.size(); i++) {
        int category = training_references[i].y;
        if (feature_flip_index == phi.get_feature_index()) {
            // Boolean feature, no point in evaluating because we can't determine anything given feature poisoning.
            // Increment both counts.
            d_ptr = &(ret.second);
            d_ptr->counts[category]++;
            d_ptr = &(ret.first);
            d_ptr->counts[category]++;
        }
        else {
            std::optional<bool> result = phi.evaluate(training_references[i].x, false);
            // Boolean feature always returns true or false from evaluate
            d_ptr = result.value() ? &(ret.second) : &(ret.first);
            d_ptr->counts[category]++;
        }
    }

    // Ensure num_dropouts are well-defined XXX this is more complicated in the 3-valued case
    // except that it's not---again, there should be an invariant that we never fall into
    // maybe cases in this portion of the code
    std::vector<DropoutCounts*> iters = {&(ret.first), &(ret.second)};
    for(auto i = iters.begin(); i != iters.end(); i++) {
        int total_ct = std::accumulate((*i)->counts.cbegin(), (*i)->counts.cend(), 0);
        (*i)->num_dropout = std::min(num_dropout, total_ct);
        // correct for one-sided cases for extra precision. Not needed for soundness.
        (*i)->num_labels_flip = std::min(num_labels_flip, total_ct);
        (*i)->label_sens_info = label_sens_info;
        (*i)->num_add = num_add;
        (*i)->add_sens_info = add_sens_info;
        (*i)->num_features_flip = std::min(num_features_flip, total_ct);
        (*i)->feature_flip_index = feature_flip_index;
        (*i)->feature_flip_amt = feature_flip_amt;
    }
    return ret;
}

TrainingReferencesWithDropout TrainingReferencesWithDropout::pureSetRestriction(std::list<int> pure_possible_classes) const {
    DataReferences training_copy = training_references;
    int num_removed = 0;
    for(unsigned int i = 0; i < training_copy.size(); i++) {
        const int current_y = training_copy[i].y;
        if(std::none_of(pure_possible_classes.cbegin(), pure_possible_classes.cend(),
                    [&current_y](int y) { return y == current_y; })) {
            training_copy.remove(i);
            num_removed++;
            i--;
        }
    }
    // We will only call this when it's guaranteed to be non-trivial,
    // so we need not check that num_removed <= num_dropout
    return TrainingReferencesWithDropout(training_copy, num_dropout - num_removed, num_add, add_sens_info, num_labels_flip, label_sens_info, num_features_flip, feature_flip_index, feature_flip_amt);
}

TrainingReferencesWithDropout TrainingReferencesWithDropout::filter(const SymbolicPredicate &phi, bool positive_flag) const {
    // Note: I would expect to end up here only in the abstract (box) case, but we also end up here for -V via filterAndUnion
    TrainingReferencesWithDropout ret(*this);
    bool remove;
    std::optional<bool> result;
    int num_maybes = 0;
    int feature_index = phi.get_feature_index();
    for(unsigned int i = 0; i < ret.training_references.size(); i++) {
       if (ret.feature_flip_index == feature_index) {
            result = phi.evaluate(ret.training_references[i].x, true, ret.feature_flip_amt);

            // We return {} when x in [lb-1, ub+1] - we might include it, but might not
            if (!result.has_value()) {
                num_maybes++;
            } else if (positive_flag != result.value()) {
                ret.training_references.remove(i);
                i--;
            }
        }
        else
        {
            result = phi.evaluate(ret.training_references[i].x, false, 0);
            if (!result.has_value()) {
                num_maybes++;
            }
            remove = result.has_value() && (positive_flag != result.value());
            if (remove) {
                ret.training_references.remove(i);
                i--;
            }
        }
        
    }
    // We don't know whether the 'maybe' points are in the dataset - so we might have to drop them,
    // which is why we increase n. We don't have to increase # of labels of features to flip.
    ret.num_dropout = std::min(ret.num_dropout + num_maybes, (int)ret.training_references.size());
    ret.num_labels_flip = std::min(ret.num_labels_flip, (int)ret.training_references.size());
    ret.num_features_flip = std::min(ret.num_features_flip, (int)ret.training_references.size());
    return ret;
}

/**
 * TrainingSetDropoutDomain members
 */

// What is happening here?? isBottomElement=training set is empty
TrainingReferencesWithDropout TrainingSetDropoutDomain::meetImpurityEqualsZero(const TrainingReferencesWithDropout &element) const {
    if(isBottomElement(element)) {
        return element;
    }
    std::vector<int> counts = element.baseCounts();
    std::list<int> pure_possible_classes(0);
    // could adjust for one-sided for additional precision, not needed for soundness
    for(unsigned int i = 0; i < counts.size(); i++) {
        // It's possible that all but the i-class elements could be removed (??? what does this comment mean?)
        // We could drop or flip the label on all elements not in this class, i.e. it's possible that 100% of elements belong to this class
        if(element.training_references.size() - counts[i] <= (element.num_dropout + element.num_labels_flip)) {
            // TO DO ONE-sided: this is nly true if the remaining counts have correct one-sided feature val
            pure_possible_classes.push_back(i);
        }
    }

    // If no pure class is possible, we return a bottom element 
    if(pure_possible_classes.size() == 0) {
        return TrainingReferencesWithDropout();
    }
    // If any number of classes are possible, our abstraction is too coarse
    // to do better than returning the restriction to just those classes,
    // so we soundly overapproximate by doing exactly that
    else if(pure_possible_classes.size() == counts.size()) {
        return element;
    } else {
        return element.pureSetRestriction(pure_possible_classes);
    }
}

TrainingReferencesWithDropout TrainingSetDropoutDomain::meetImpurityNotEqualsZero(const TrainingReferencesWithDropout &element) const {
    // Our abstraction is not capable of expressing this precisely
    return element;
}

bool TrainingSetDropoutDomain::isBottomElement(const TrainingReferencesWithDropout &element) const {
    return element.training_references.size() == 0;
}

TrainingReferencesWithDropout TrainingSetDropoutDomain::binary_join(const TrainingReferencesWithDropout &e1, const TrainingReferencesWithDropout &e2) const {
    if(isBottomElement(e1)) {
        return e2;
    } else if(isBottomElement(e2)) {
        return e1;
    }
    DataReferences d = DataReferences::set_union(e1.training_references, e2.training_references);
    int n1 = d.size() - e2.training_references.size() + e2.num_dropout; // Note |(T1 U T2) \ T1| = |T2 \ T1|
    int n2 = d.size() - e1.training_references.size() + e1.num_dropout;
    int new_labels = std::max(e1.num_labels_flip, e2.num_labels_flip);
    int new_add = std::max(e1.num_add, e2.num_add);
    int new_flip = std::max(e1.num_features_flip, e2.num_features_flip);
    return TrainingReferencesWithDropout(d, std::max(n1, n2), new_add, e1.add_sens_info, new_labels, e1.label_sens_info,  new_flip, e1.feature_flip_index, e1.feature_flip_amt);
}

/**
 * PredicateSetDomain members
 */

PredicateAbstraction PredicateSetDomain::meetPhiIsBottom(const PredicateAbstraction &element) const {
    bool contains_bottom = std::any_of(element.cbegin(), element.cend(),
            [](std::optional<SymbolicPredicate> phi){ return !phi.has_value(); });
    if(contains_bottom) {
        std::optional<SymbolicPredicate> bottom = {};
        return PredicateAbstraction({bottom}); // The vector of a single bottom element.
    } else {
        return PredicateAbstraction(); // The empty vector
    }
}

PredicateAbstraction PredicateSetDomain::meetPhiIsNotBottom(const PredicateAbstraction &element) const {
    PredicateAbstraction phis(element); // Make a copy
    // Iterate over the copy and delete any bottom element
    for(auto i = phis.begin(); i != phis.end(); i++) {
        if(!i->has_value()) { // It should be an invariant of the code that this only happens once, but...
            phis.erase(i--);
        }
    }
    return phis;
}

PredicateAbstraction PredicateSetDomain::meetXModelsPhi(const PredicateAbstraction &element, const FeatureVector &x) const {
    // at this point we're not building the model, we're just trying to classify x. So, sometimes we don't know what branch x 
    // goes down (if it's in (lb,ub), but this doesn't depend on feature poisoning
    if(isBottomElement(element)) {
        return element;
    }
    PredicateAbstraction phis;
    for(auto i = element.cbegin(); i != element.cend(); i++) {
        // The grammar should enforce that we always have i->has_value()
        std::optional<bool> result = i->value().evaluate(x, false);
        // With three-valued logic, we include "maybe" (!result.has_value())
        if(!result.has_value() || result.value()) {
            phis.push_back(*i);
        }
    }
    return phis;
}

PredicateAbstraction PredicateSetDomain::meetXNotModelsPhi(const PredicateAbstraction &element, const FeatureVector &x) const {
    if(isBottomElement(element)) {
        return element;
    }
    PredicateAbstraction phis;
    for(auto i = element.cbegin(); i != element.cend(); i++) {
        // The grammar should enforce that we always have i->has_value()
        std::optional<bool> result = i->value().evaluate(x, false);
        // With three-valued logic, we include "maybe" (!result.has_value())
        if(!result.has_value() || !result.value()) { // This is the only line that differs from meetXModelsPhi
            phis.push_back(*i);
        }
    }
    return phis;
}

bool PredicateSetDomain::isBottomElement(const PredicateAbstraction &element) const {
    return element.size() == 0;
}

PredicateAbstraction PredicateSetDomain::binary_join(const PredicateAbstraction &e1, const PredicateAbstraction &e2) const {
    if(isBottomElement(e1)) {
        return e2;
    } else if(isBottomElement(e2)) {
        return e1;
    }
    // TODO take advantage of algorithm's std::set_union
    PredicateAbstraction phis = e1; // Make a copy
    for(auto i = e2.cbegin(); i != e2.cend(); i++) {
        if(std::none_of(phis.cbegin(), phis.cend(),
                    [&i](std::optional<SymbolicPredicate> j){ return *i == *j; })) {
            phis.push_back(*i);
        }
    }
    return phis;
}

/**
 * PosteriorDistributionIntervalDomain members
 */

bool PosteriorDistributionIntervalDomain::isBottomElement(const PosteriorDistributionAbstraction &element) const {
    // XXX a better check would be if the intervals for each category
    // actually admit some concretization that forms a probability distribution.
    return element.size() == 0; // This is just based off of the default empty constructor
}

PosteriorDistributionAbstraction PosteriorDistributionIntervalDomain::binary_join(const PosteriorDistributionAbstraction &e1, const PosteriorDistributionAbstraction &e2) const {
    if(isBottomElement(e1)) {
        return e2;
    } else if(isBottomElement(e2)) {
        return e1;
    }
    // XXX strong assumption (invariant?) that the two categorical dist's have the same size
    PosteriorDistributionAbstraction ret(e1.size());
    for(unsigned int i = 0; i < e1.size(); i++) {
        ret[i] = Interval<double>::join(e1[i], e2[i]);
    }
    return ret;
}

/**
 * BoxDropoutDomain members
 */

// First two auxiliary methods

inline bool couldBeEmpty(const TrainingReferencesWithDropout::DropoutCounts &counts) {
    // question, for feature manipulation is this different? Because we could manipulate the feature of elements to move them to other branch?
    // would also mean mustBeEmpty is always false when we can change feat. that phi looks at
    return std::accumulate(counts.counts.begin(), counts.counts.end(), 0) <= counts.num_dropout;
}

inline bool mustBeEmpty(const TrainingReferencesWithDropout::DropoutCounts &counts) {
    if (counts.num_add > 0) {
        return false;
    }
    return std::accumulate(counts.counts.begin(), counts.counts.end(), 0) == 0;
}

void BoxDropoutDomain::computePredicatesAndScores(std::list<ScoreEntry> &exists_nontrivial, std::list<const ScoreEntry *> &forall_nontrivial, const TrainingReferencesWithDropout &training_set_abstraction, int feature_index) const {
    switch(training_set_abstraction.training_references.getFeatureTypes()[feature_index]) {
        // XXX need to make changes here if adding new feature types
        case FeatureType::BOOLEAN:
            computeBooleanFeaturePredicateAndScore(exists_nontrivial, forall_nontrivial, training_set_abstraction, feature_index);
            break;
        case FeatureType::NUMERIC:
            computeNumericFeaturePredicatesAndScores(exists_nontrivial, forall_nontrivial, training_set_abstraction, feature_index);
            break;
    }
}

void BoxDropoutDomain::computeBooleanFeaturePredicateAndScore(std::list<ScoreEntry> &exists_nontrivial, std::list<const ScoreEntry *> &forall_nontrivial, const TrainingReferencesWithDropout &training_set_abstraction, int feature_index) const {
    SymbolicPredicate phi(feature_index);
    // all of our features are numeric, so not applicable, but could improve precision here (not necessary for soundness)
    auto counts = training_set_abstraction.splitCounts(phi);
    // TO DO  - update num_labels_flip here because Joint Impurity expects it to be one-sided up to date
    if(!mustBeEmpty(counts.first) && !mustBeEmpty(counts.second)) {
        Interval<double> temp = jointImpurity(counts.first.counts,
                                              counts.first.num_dropout, counts.first.num_add, counts.first.num_labels_flip, counts.first.num_features_flip,
                                              counts.second.counts,
                                              counts.second.num_dropout, counts.second.num_add, counts.second.num_labels_flip, counts.second.num_features_flip,
                                              training_set_abstraction.label_sens_info, training_set_abstraction.add_sens_info);
        exists_nontrivial.push_back(std::make_pair(phi, temp));
        if(!couldBeEmpty(counts.first) && !couldBeEmpty(counts.second)) {
            forall_nontrivial.push_back(&exists_nontrivial.back());
        }
    }
}

void BoxDropoutDomain::computeNumericFeaturePredicatesAndScores(std::list<ScoreEntry> &exists_nontrivial, std::list<const ScoreEntry *> &forall_nontrivial, const TrainingReferencesWithDropout &training_set_abstraction, int feature_index) const {
    // 0 is value of item that phi looks at, 1 is label, 2 is value of label-flipping target, 3 is value of adding target
    std::vector<std::tuple<float,int, int, int>> value_class_pairs(training_set_abstraction.training_references.size());

    // Access the data, only looking at the feature that is relevant for this predicate and for one-sided data poisoning
    for(unsigned int j = 0; j < training_set_abstraction.training_references.size(); j++) {
        DataRow temp = training_set_abstraction.training_references[j];
       std:: get<0>(value_class_pairs[j]) = temp.x[feature_index].getNumericValue();
       std::get<1>(value_class_pairs[j]) = temp.y;
        if (training_set_abstraction.label_sens_info.first > -1) {
            std::get<2>(value_class_pairs[j]) = temp.x[training_set_abstraction.label_sens_info.first].getNumericValue();
        }
        if (training_set_abstraction.add_sens_info.first > -1) {
            std::get<3>(value_class_pairs[j]) = temp.x[training_set_abstraction.add_sens_info.first].getNumericValue();
        }
    }
    if(value_class_pairs.size() < 2) {
        return;
    }
    
    std::sort(value_class_pairs.begin(), value_class_pairs.end(),
              [](const std::tuple<float,int, int, int> &p1, const std::tuple<float,int, int, int> &p2)
              { return std::get<0>(p1) < std::get<0>(p2); } );
    std::pair<TrainingReferencesWithDropout::DropoutCounts, TrainingReferencesWithDropout::DropoutCounts> split_counts = {
            { std::vector<int>(training_set_abstraction.training_references.getNumCategories(), 0), 0 },
            { training_set_abstraction.baseCounts(), training_set_abstraction.num_dropout, training_set_abstraction.num_add,
                training_set_abstraction.add_sens_info, training_set_abstraction.num_labels_flip, training_set_abstraction.label_sens_info,
                training_set_abstraction.num_features_flip, training_set_abstraction.feature_flip_index, training_set_abstraction.feature_flip_amt } };
    
    //std::pair<std::vector<int>, std::vector<int>> split_counts_targeted = {
      //  {std::vector<int>(training_set_abstraction.training_references.getNumCategories(), 0)},
      //  {std::vector<int>(training_set_abstraction.training_referenes.getNumCategories(), 0)}
    //};

    int num_label_first = 0;
    int num_label_second = 0;

    // iterating through training data elements    
    if (feature_index == training_set_abstraction.feature_flip_index) {
        std::list<SymbolicPredicate> phis_to_add = {};
        float feat_flip_amt = training_set_abstraction.feature_flip_amt;
        std::list<float> vals_of_phi = {};

        for (auto i = value_class_pairs.begin(); i + 1 != value_class_pairs.end(); i++) {
            if (training_set_abstraction.label_sens_info.first > -1) {
                if (std::get<2>(*i) == training_set_abstraction.label_sens_info.second) {
                    num_label_second += 1;
                }
            }
            // If value of this and same match, just continue to next one
            if(std::get<0>(*i) == std::get<0>(*(i+1))) {
                continue;
            }

            vals_of_phi.push_back(std::get<0>(*i) - feat_flip_amt);
            vals_of_phi.push_back(std::get<0>(*i));
            vals_of_phi.push_back(std::get<0>(*i) + feat_flip_amt);

            // this is a lil weird but I think keeping (i+1)->first makes sense because there's not a great alternative
            // hmm, this could cause weirdness in the next section if i->first + 1 > (i+1)->first or similar for lower bound
            phis_to_add.push_back(SymbolicPredicate(feature_index, std::get<0>(*i) - feat_flip_amt, std::get<0>(*(i+1))));
            phis_to_add.push_back(SymbolicPredicate(feature_index, std::get<0>(*i) + feat_flip_amt, std::max(std::get<0>(*(i+1)), std::get<0>(*i) + feat_flip_amt)));

            // Otherwise, add phi to list to (eventually) add
            phis_to_add.push_back(SymbolicPredicate(feature_index, std::get<0>(*i), std::get<0>(*(i+1))));
        }

        vals_of_phi.push_back(std::get<0>(*(value_class_pairs.end())) - feat_flip_amt);
        vals_of_phi.push_back(std::get<0>(*(value_class_pairs.end())));
        vals_of_phi.push_back(std::get<0>(*(value_class_pairs.end())) + feat_flip_amt);

        vals_of_phi.sort([](const float a, const float b) { return a < b; });

        while (vals_of_phi.size() > 1) {
            float val = vals_of_phi.front();
            vals_of_phi.pop_front();
            float val_next = vals_of_phi.front();
            if (val == val_next) {
                continue;
            }

            phis_to_add.push_back(SymbolicPredicate(feature_index, val, val_next));
        }

        int start = 0;
        int last_removed = 0;

        int lb; int ub; int num_dropout_incr_first; int num_dropout_incr_second;

        while (!phis_to_add.empty()) {
            int two_less = 0;

            SymbolicPredicate phi = phis_to_add.front();
            phis_to_add.pop_front();
            lb = phi.get_lb();
            ub = phi.get_ub();

            int num_dropout_iffy_first = 0; int num_dropout_iffy_second = 0;
            int num_dropout_inclusive_first = 0; int num_dropout_inclusive_second = 0;

            // Calculate the number in our iffy range [lb-1, lb+1)
            for (int i = two_less; i< value_class_pairs.size(); i++) {
                auto item = value_class_pairs[i];
                // n will be the total number of superfluous elements
                // for split_counts.first, all elements in (lb, lb+f] are potentially superfluous and up to f
                // elements in (B-fa, B] are superfluous. Flipped for split_counts.second.
                if (std::get<0>(item) < lb - feat_flip_amt) {
                    two_less += 1; // we won't have to consider this index again, next phi lb will be higher
                } else if (std::get<0>(item) < lb) {
                    if (num_dropout_inclusive_first < training_set_abstraction.num_features_flip) {
                        num_dropout_inclusive_first += 1;
                    }
                    num_dropout_iffy_second += 1;
                }
                else if (std::get<0>(item) < lb + feat_flip_amt) {
                    if (num_dropout_inclusive_second < training_set_abstraction.num_features_flip) {
                        num_dropout_inclusive_second += 1;
                    }
                    num_dropout_iffy_first += 1;
                } else {
                    break;
                }
            }
            
            num_dropout_incr_first = num_dropout_iffy_first + num_dropout_inclusive_first;
            num_dropout_incr_second = num_dropout_iffy_second + num_dropout_inclusive_second;

            // For anything in [start, lb+1), add to split_counts.first
            for (int i=start; i<value_class_pairs.size(); i++) {
                auto item = value_class_pairs[i];
                if (std::get<0>(item) < lb + feat_flip_amt) {
                    split_counts.first.counts[std::get<1>(item)]++;
                    start += 1;

                    if (training_set_abstraction.label_sens_info.first > -1) {
                        if (std::get<2>(item) == training_set_abstraction.label_sens_info.second) {
                            num_label_first += 1;
                        }
                    }
                } else {
                    break;
                }
            }

            // For anything in [last_removed, lb-1), remove it from split_counts.second
            for (int i=last_removed; i<start; i++) {
                auto item = value_class_pairs[i];
                if (std::get<0>(item) < lb - feat_flip_amt) {
                    split_counts.second.counts[std::get<1>(item)]--;
                    last_removed += 1;

                    if (training_set_abstraction.label_sens_info.first > -1) {
                        if (std::get<2>(item) == training_set_abstraction.label_sens_info.second) {
                            num_label_second -= 1;
                        }
                    }
                } else {
                    break;
                }
            }

            // update n, l, and f
            // need to do - special one-sided addition case: if phi looks at feature, should set to 0 on one branch
            // ^ would add more precision, but not needed for soundness so I'm leaving for now - hard to mesh = with phi's <
            int remaining = std::accumulate(split_counts.first.counts.cbegin(), split_counts.first.counts.cend(), 0);
            split_counts.first.num_dropout = std::min(remaining, training_set_abstraction.num_dropout + num_dropout_incr_first);
            if ((training_set_abstraction.label_sens_info.first > -1) && (split_counts.first.num_labels_flip < num_label_first) && (split_counts.first.num_labels_flip < training_set_abstraction.num_labels_flip)) {
                split_counts.first.num_labels_flip++;
            } else if (split_counts.first.num_labels_flip < training_set_abstraction.num_labels_flip) {
                split_counts.first.num_labels_flip++;
            }

            if (split_counts.first.num_features_flip < training_set_abstraction.num_features_flip) {
                split_counts.first.num_features_flip++;
            }

            remaining = std::accumulate(split_counts.second.counts.cbegin(), split_counts.second.counts.cend(), 0);
            split_counts.second.num_dropout = std::min(remaining, training_set_abstraction.num_dropout + num_dropout_incr_second);
            if ((training_set_abstraction.label_sens_info.first > -1 ) && (num_label_second < split_counts.second.num_labels_flip)) {
                split_counts.second.num_labels_flip = num_label_second;
            } else if (remaining < split_counts.second.num_labels_flip) {
                split_counts.second.num_labels_flip = remaining;
            }

            if (remaining < split_counts.second.num_features_flip) {
                split_counts.second.num_features_flip = remaining;
            }

            Interval<double> temp = jointImpurity(split_counts.first.counts,
                                        split_counts.first.num_dropout, split_counts.first.num_add,
                                        split_counts.first.num_labels_flip, split_counts.first.num_features_flip,
                                        split_counts.second.counts,
                                        split_counts.second.num_dropout, split_counts.second.num_add,
                                        split_counts.second.num_labels_flip, split_counts.second.num_features_flip,
                                        training_set_abstraction.label_sens_info, training_set_abstraction.add_sens_info);
            exists_nontrivial.push_back(std::make_pair(phi, temp));
            if (!couldBeEmpty(split_counts.first) && !couldBeEmpty(split_counts.second)) {
                forall_nontrivial.push_back(&exists_nontrivial.back());
            }
        }
    }
    else {
        if (training_set_abstraction.label_sens_info.first > -1) {
            for (auto i = value_class_pairs.begin(); i != value_class_pairs.end(); i++) {
                if (std::get<2>(*i) == training_set_abstraction.label_sens_info.second) {
                    num_label_second += 1;
                }
            }
        }
        
        for(auto i = value_class_pairs.begin(); i + 1 != value_class_pairs.end(); i++) {

            split_counts.first.counts[std::get<1>(*i)]++;
            if ((training_set_abstraction.label_sens_info.first > -1) && (std::get<2>(*i) == training_set_abstraction.label_sens_info.second) ) {
                num_label_first += 1;
                num_label_second -= 1;
            }

            if(split_counts.first.num_dropout < training_set_abstraction.num_dropout) {
                split_counts.first.num_dropout++;
            }

            if ((training_set_abstraction.label_sens_info.first > -1) && (split_counts.first.num_labels_flip < num_label_first) && (split_counts.first.num_labels_flip < training_set_abstraction.num_labels_flip)) {
                split_counts.first.num_labels_flip++;
            } else if (split_counts.first.num_labels_flip < training_set_abstraction.num_labels_flip) {
                split_counts.first.num_labels_flip++;
            }

            if (split_counts.first.num_features_flip < training_set_abstraction.num_features_flip) {
                split_counts.second.num_features_flip++;
            }

            split_counts.second.counts[std::get<1>(*i)]--;
            int remaining = std::accumulate(split_counts.second.counts.cbegin(), split_counts.second.counts.cend(), 0);
            if (remaining < split_counts.second.num_dropout) {
                split_counts.second.num_dropout=remaining;
            }
            if ((training_set_abstraction.label_sens_info.first > -1) && (num_label_second < split_counts.second.num_labels_flip)) {
                split_counts.second.num_labels_flip = num_label_second;
            } else if (remaining < split_counts.second.num_labels_flip) {
                split_counts.second.num_labels_flip = remaining;
            }

            if (remaining < split_counts.second.num_features_flip) {
                split_counts.second.num_features_flip = remaining;
            }

            if(std::get<0>(*i) == std::get<0>(*(i+1))) {
                continue;
            }

            // At this point, the check for if we should include in exists_nontrivial would always pass.
            // For each adjacent pair (l,u) store a symbolic predicate x<=[l,u)
            SymbolicPredicate phi(feature_index, std::get<0>(*i), std::get<0>(*(i+1)));
            Interval<double> temp = jointImpurity(split_counts.first.counts,
                                        split_counts.first.num_dropout, split_counts.first.num_add,
                                        split_counts.first.num_labels_flip, split_counts.first.num_features_flip,
                                        split_counts.second.counts,
                                        split_counts.second.num_dropout, split_counts.second.num_add,
                                        split_counts.second.num_labels_flip, split_counts.second.num_features_flip,
                                        training_set_abstraction.label_sens_info, training_set_abstraction.add_sens_info);
            exists_nontrivial.push_back(std::make_pair(phi, temp));
            if (!couldBeEmpty(split_counts.first) && !couldBeEmpty(split_counts.second)) {
                forall_nontrivial.push_back(&exists_nontrivial.back());
            }
        }
    }
}

PredicateAbstraction BoxDropoutDomain::bestSplit(const TrainingReferencesWithDropout &training_set_abstraction) const {
    std::list<ScoreEntry> exists_nontrivial;
    std::list<const ScoreEntry *> forall_nontrivial; // Points to elements in exists_nontrivial
    for(int i = 0; i < training_set_abstraction.training_references.getFeatureTypes().size(); i++) {
        computePredicatesAndScores(exists_nontrivial, forall_nontrivial, training_set_abstraction, i);
    }

    if(forall_nontrivial.size() == 0) {
        PredicateAbstraction ret(exists_nontrivial.size() + 1);
        int index = 0;
        for(auto i = exists_nontrivial.cbegin(); i != exists_nontrivial.cend(); i++,index++) {
            ret[index] = i->first;
        }
        ret[exists_nontrivial.size()] = {}; // The non option type value
        return ret;
    } else {
        // Find the threshold using only predicates from forall_nontrivial
        double min_upper_bound; // Always gets initialized unless forall_nontrivial.size() == 0,
                                // and we prior have an if statement to check that
        for(auto i = forall_nontrivial.cbegin(); i != forall_nontrivial.cend(); i++) {
            if(i == forall_nontrivial.begin() || min_upper_bound > (*i)->second.get_upper_bound()) {
                min_upper_bound = (*i)->second.get_upper_bound();
            }
        }
        // Return any predicates in exists_nontrivial whose score could beat the threshold
        PredicateAbstraction ret;
        for(auto i = exists_nontrivial.cbegin(); i != exists_nontrivial.cend(); i++) {
            if(i->second.get_lower_bound() <= min_upper_bound) {
                ret.push_back(i->first);
            }
        }
        return ret;
    }
}

TrainingReferencesWithDropout BoxDropoutDomain::filter(const TrainingReferencesWithDropout &training_set_abstraction, const PredicateAbstraction &predicate_abstraction) const {
    // We only use filter in the abstract (box) case
    std::vector<TrainingReferencesWithDropout> joins;
    for(auto i = predicate_abstraction.cbegin(); i != predicate_abstraction.cend(); i++) {
        // The grammar should enforce that each i->has_value()
        TrainingReferencesWithDropout temp = training_set_abstraction.filter(i->value(), true);
        joins.push_back(temp);
    }
    return training_set_domain->join(joins);
}

TrainingReferencesWithDropout BoxDropoutDomain::filterNegated(const TrainingReferencesWithDropout &training_set_abstraction, const PredicateAbstraction &predicate_abstraction) const {
    // XXX copy and pasted previous method with one change; refactor with common method
    std::vector<TrainingReferencesWithDropout> joins;
    for(auto i = predicate_abstraction.cbegin(); i != predicate_abstraction.cend(); i++) {
        // The grammar should enforce that each i->has_value()
        TrainingReferencesWithDropout temp = training_set_abstraction.filter(i->value(), false);
        joins.push_back(temp);
    }
    return training_set_domain->join(joins);
}

PosteriorDistributionAbstraction BoxDropoutDomain::summary(const TrainingReferencesWithDropout &training_set_abstraction) const {
    return estimateCategorical(training_set_abstraction.baseCounts(), training_set_abstraction.num_dropout, training_set_abstraction.num_add,
                            training_set_abstraction.num_labels_flip, training_set_abstraction.num_features_flip, 
                            training_set_abstraction.label_sens_info, training_set_abstraction.add_sens_info);
}
