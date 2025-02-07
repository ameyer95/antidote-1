#include "CommonEnums.h"
#include <map>

template <typename T>
std::set<std::string> gather_enum_strings(const std::map<T, std::string> &enum_strings) {
    std::set<std::string> ret;
    for(auto i = enum_strings.cbegin(); i != enum_strings.cend(); i++) {
        ret.insert(i->second);
    }
    return ret;
}


/**
 * Code for ExperimentDataEnum
 */

const std::map<ExperimentDataEnum, std::string> experiment_data_enum_strings = {
    {ExperimentDataEnum::MNIST_BOOLEAN_1_7, "mnist_simple_1_7"},
    {ExperimentDataEnum::MNIST_1_7, "mnist_1_7"},
    {ExperimentDataEnum::MNIST_BOOLEAN, "mnist_simple"},
    {ExperimentDataEnum::MNIST, "mnist"},
    {ExperimentDataEnum::UCI_IRIS, "iris"},
    {ExperimentDataEnum::UCI_CANCER, "cancer"},
    {ExperimentDataEnum::UCI_WINE, "wine"},
    {ExperimentDataEnum::UCI_WINE_2CLASS, "wine_2"},
    {ExperimentDataEnum::UCI_YEAST, "yeast"},
    {ExperimentDataEnum::UCI_RETINOPATHY, "retinopathy"},
    {ExperimentDataEnum::UCI_MAMMOGRAPHY, "mammography"},
    {ExperimentDataEnum::ADULT_INCOME, "adult_income"},
    {ExperimentDataEnum::GERMAN_LOAN, "german_loan"},
    {ExperimentDataEnum::COMPAS, "compas"},
    {ExperimentDataEnum::DRUG_CONSUMPTION, "drug_consumption"},
};

std::string to_string(const ExperimentDataEnum &e) {
    return experiment_data_enum_strings.at(e);
}

ExperimentDataEnum string_to_ExperimentDataEnum(const std::string &s) {
    for(auto i = experiment_data_enum_strings.cbegin(); i != experiment_data_enum_strings.cend(); i++) {
        if(i->second == s) {
            return i->first;
        }
    }
    // XXX not handling incorrect inputs
}

std::set<std::string> strings_of_ExperimentDataEnum() {
    return gather_enum_strings(experiment_data_enum_strings);
}


/**
 * Code for DisjunctsMergeMode
 */

const std::map<DisjunctsMergeMode, std::string> disjuncts_merge_mode_strings = {
    {DisjunctsMergeMode::GREEDY, "greedy"},
    {DisjunctsMergeMode::OPTIMAL, "optimal"},
};

std::string to_string(const DisjunctsMergeMode &mode) {
    return disjuncts_merge_mode_strings.at(mode);
}

DisjunctsMergeMode string_to_DisjunctsMergeMode(const std::string &s) {
    for(auto i = disjuncts_merge_mode_strings.cbegin(); i != disjuncts_merge_mode_strings.cend(); i++) {
        if(i->second == s) {
            return i->first;
        }
    }
    // XXX not handling incorrect inputs
}

std::set<std::string> strings_of_DisjunctsMergeMode() {
    return gather_enum_strings(disjuncts_merge_mode_strings);
}
