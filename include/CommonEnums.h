#ifndef COMMONENUMS_H
#define COMMONENUMS_H

/**
 * Classes that run actual code have enums to select options,
 * but, for example, the argument parser needs to know those enums
 * as well as convert to them from strings.
 * This file is an attempt to centralize those definitions.
 */

#include <set>
#include <string>


/**
 * Experiment code can request to fetch a variety of datasets
 */

enum class ExperimentDataEnum {
    MNIST_BOOLEAN_1_7,
    MNIST,
    UCI_IRIS,
    UCI_CANCER,
    UCI_WINE,
};

std::string to_string(const ExperimentDataEnum &e);
ExperimentDataEnum string_to_ExperimentDataEnum(const std::string &s);
std::set<std::string> strings_of_ExperimentDataEnum();


/**
 * The bounded disjuncts domain has options for the merging strategy
 */

enum class DisjunctsMergeMode { GREEDY, OPTIMAL };

std::string to_string(const DisjunctsMergeMode &mode);
DisjunctsMergeMode string_to_DisjunctsMergeMode(const std::string &s);
std::set<std::string> strings_of_DisjunctsMergeMode();

#endif
