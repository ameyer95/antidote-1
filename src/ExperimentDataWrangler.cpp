#include "ExperimentDataWrangler.h"
#include "Feature.hpp"
#include <string>
#include <vector>

/**
 * Non-member auxiliary functions
 */

inline std::string int_to_MNIST_label(int label) {
    switch(label) {
        case 0:
            return "zero";
        case 1:
            return "one";
        case 2:
            return "two";
        case 3:
            return "three";
        case 4:
            return "four";
        case 5:
            return "five";
        case 6:
            return "six";
        case 7:
            return "seven";
        case 8:
            return "eight";
        case 9:
            return "nine";
        default:
            return "???"; // XXX shouldn't happen
    }
}

// Uses 128 as the binary threshold (each pixel is a byte)
inline FeatureVector Image_to_Input(const Image &image) {
    FeatureVector ret(MNIST_IMAGE_SIZE);
    for(int i = 0; i < MNIST_IMAGE_SIZE; i++) {
        ret[i] = *(image + i) > 128;
    }
    return ret;
}

// The classes pair assigns the labels first -> 0 and second -> 1
DataSet* simplifiedMNIST(const RawMNIST &mnist, const std::pair<int, int> &classes) {
    DataSet *ret = new DataSet { FeatureVectorHeader(MNIST_IMAGE_SIZE, FeatureType::BOOLEAN),
                                 2,
                                 std::vector<DataRow>(0) };

    for(unsigned int i = 0; i < mnist.size(); i++) {
        if(mnist[i].second == classes.first || mnist[i].second == classes.second) {
            DataRow temp = { Image_to_Input(mnist[i].first),
                             (mnist[i].second == classes.second ? 1 : 0) };
            ret->rows.push_back(temp);
        }
    }

    return ret;
}

/**
 * ExperimentDataWrangler member functions
 */

ExperimentDataWrangler::ExperimentDataWrangler(const std::string &path_prefix) {
    this->path_prefix = path_prefix;
}

ExperimentDataWrangler::~ExperimentDataWrangler() {
    for(auto i = cache.begin(); i != cache.end(); i++) {
        delete i->second;
    }
}

void ExperimentDataWrangler::loadData(const ExperimentDataEnum &dataset) {
    switch(dataset) {
        case ExperimentDataEnum::MNIST_BOOLEAN_1_7:
            cache.insert(std::make_pair(dataset, loadSimplifiedMNIST(std::make_pair(1,7))));
            break;
        // XXX TODO all the other cases
    }
}

ExperimentData* ExperimentDataWrangler::loadSimplifiedMNIST(const std::pair<int, int> &classes) {
    RawMNIST raw_mnist_training(MNISTMode::TRAINING, path_prefix);
    RawMNIST raw_mnist_test(MNISTMode::TEST, path_prefix);
    ExperimentData *mnist = new ExperimentData;
    mnist->training = simplifiedMNIST(raw_mnist_training, classes);
    mnist->test = simplifiedMNIST(raw_mnist_test, classes);
    mnist->class_labels = {int_to_MNIST_label(classes.first), int_to_MNIST_label(classes.second)};
    return mnist;
}

const ExperimentData* ExperimentDataWrangler::fetch(const ExperimentDataEnum &dataset) {
    if(cache.find(dataset) != cache.cend()) {
        return cache.at(dataset);
    } else {
        loadData(dataset);
        return cache.at(dataset);
    }
}
