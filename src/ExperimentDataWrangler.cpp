#include "ExperimentDataWrangler.h"
#include "Feature.hpp"
#include "MNIST.h"
#include "UCI.h"
#include <map>
#include <string>
#include <utility>
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
inline FeatureVector Image_to_bools(const Image &image) {
    FeatureVector ret(MNIST_IMAGE_SIZE);
    for(int i = 0; i < MNIST_IMAGE_SIZE; i++) {
        ret[i] = *(image + i) > 128;
    }
    return ret;
}

inline FeatureVector Image_to_floats(const Image &image) {
    FeatureVector ret(MNIST_IMAGE_SIZE);
    for(int i = 0; i < MNIST_IMAGE_SIZE; i++) {
        ret[i] = (float)*(image + i);
    }
    return ret;
}

// The classes pair assigns the labels first -> 0 and second -> 1
DataSet* twoClassMNIST(const RawMNIST &mnist, const std::pair<int, int> &classes, bool booleanized) {
    DataSet *ret;
    if(booleanized) {
        ret = new DataSet { FeatureVectorHeader(MNIST_IMAGE_SIZE, FeatureType::BOOLEAN),
                            2,
                            std::vector<DataRow>(0) };
    } else {
        ret = new DataSet { FeatureVectorHeader(MNIST_IMAGE_SIZE, FeatureType::NUMERIC),
                            2,
                            std::vector<DataRow>(0) };
    }

    for(unsigned int i = 0; i < mnist.size(); i++) {
        if(mnist[i].second == classes.first || mnist[i].second == classes.second) {
            if(booleanized) {
                DataRow temp = { Image_to_bools(mnist[i].first),
                                 (mnist[i].second == classes.second ? 1 : 0) };
                ret->rows.push_back(temp);
            } else {
                DataRow temp = { Image_to_floats(mnist[i].first),
                                 (mnist[i].second == classes.second ? 1 : 0) };
                ret->rows.push_back(temp);
            }
        }
    }

    return ret;
}


DataSet* fullMNIST(const RawMNIST &mnist, bool booleanized) {
    DataSet *ret;
    if(booleanized) {
        ret = new DataSet { FeatureVectorHeader(MNIST_IMAGE_SIZE, FeatureType::BOOLEAN),
                            10,
                            std::vector<DataRow>(mnist.size()) };
    } else {
        ret = new DataSet { FeatureVectorHeader(MNIST_IMAGE_SIZE, FeatureType::NUMERIC),
                            10,
                            std::vector<DataRow>(mnist.size()) };
    }
    for(unsigned int i = 0; i < mnist.size(); i++) {
        if(booleanized) {
            ret->rows[i] = { Image_to_bools(mnist[i].first), mnist[i].second };
        } else {
            ret->rows[i] = { Image_to_floats(mnist[i].first), mnist[i].second };
        }
    }
    return ret;
}

void makeWineDataSetThresholded(DataSet *wine, const std::vector<std::string> &old_labels) {
    for(auto i = wine->rows.begin(); i != wine->rows.end(); i++) {
        if(stoi(old_labels[i->y]) <= 5) {
            i->y = 0;
        } else {
            i->y = 1;
        }
    }
    wine->num_categories = 2;
}

void makeWineExperimentDataThresholded(ExperimentData *wine) {
    makeWineDataSetThresholded(wine->training, wine->class_labels);
    makeWineDataSetThresholded(wine->test, wine->class_labels);
    wine->class_labels = {"low", "high"};
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
    ExperimentData *temp;
    switch(dataset) {
        case ExperimentDataEnum::MNIST_BOOLEAN_1_7:
            cache.insert(std::make_pair(dataset, loadSimplifiedMNIST(std::make_pair(1,7), true)));
            break;
        case ExperimentDataEnum::MNIST_1_7:
            cache.insert(std::make_pair(dataset, loadSimplifiedMNIST(std::make_pair(1,7), false)));
            break;
        case ExperimentDataEnum::MNIST_BOOLEAN:
            cache.insert(std::make_pair(dataset, loadFullMNIST(true)));
            break;
        case ExperimentDataEnum::MNIST:
            cache.insert(std::make_pair(dataset, loadFullMNIST(false)));
            break;
        case ExperimentDataEnum::UCI_IRIS:
            cache.insert(std::make_pair(dataset, loadUCI(UCINames::IRIS)));
            break;
        case ExperimentDataEnum::UCI_CANCER:
            cache.insert(std::make_pair(dataset, loadUCI(UCINames::CANCER)));
            break;
        case ExperimentDataEnum::UCI_WINE:
            cache.insert(std::make_pair(dataset, loadUCI(UCINames::WINE)));
            break;
        case ExperimentDataEnum::UCI_WINE_2CLASS:
            temp = loadUCI(UCINames::WINE);
            makeWineExperimentDataThresholded(temp);
            cache.insert(std::make_pair(dataset, temp));
            break;
        case ExperimentDataEnum::UCI_YEAST:
            cache.insert(std::make_pair(dataset, loadUCI(UCINames::YEAST)));
            break;
        case ExperimentDataEnum::UCI_RETINOPATHY:
            cache.insert(std::make_pair(dataset, loadUCI(UCINames::RETINOPATHY)));
            break;
        case ExperimentDataEnum::UCI_MAMMOGRAPHY:
            cache.insert(std::make_pair(dataset, loadUCI(UCINames::MAMMOGRAPHY)));
            break;
    }
}


ExperimentData* ExperimentDataWrangler::loadSimplifiedMNIST(const std::pair<int, int> &classes, bool booleanized) {
    RawMNIST raw_mnist_training(MNISTMode::TRAINING, path_prefix);
    RawMNIST raw_mnist_test(MNISTMode::TEST, path_prefix);
    ExperimentData *mnist = new ExperimentData;
    mnist->training = twoClassMNIST(raw_mnist_training, classes, booleanized);
    mnist->test = twoClassMNIST(raw_mnist_test, classes, booleanized);
    mnist->class_labels = {int_to_MNIST_label(classes.first), int_to_MNIST_label(classes.second)};
    return mnist;
}

ExperimentData* ExperimentDataWrangler::loadFullMNIST(bool booleanized) {
    RawMNIST raw_mnist_training(MNISTMode::TRAINING, path_prefix);
    RawMNIST raw_mnist_test(MNISTMode::TEST, path_prefix);
    DataSet *mnist_training, *mnist_test;
    mnist_training = fullMNIST(raw_mnist_training, booleanized);
    mnist_test = fullMNIST(raw_mnist_test, booleanized);
    ExperimentData *mnist = new ExperimentData { mnist_training, mnist_test, std::vector<std::string>(10) };
    for(int i = 0; i < 10; i++) {
        mnist->class_labels[i] = int_to_MNIST_label(i);
    }
    return mnist;
}

ExperimentData* ExperimentDataWrangler::loadUCI(const UCINames &dataset) {
    UCI raw_uci(dataset, path_prefix);
    DataSet *uci_training = new DataSet {
        FeatureVectorHeader(raw_uci.getTrainingData()[0].x.size(), FeatureType::NUMERIC), // XXX many assumptions
        (int)(raw_uci.getLabels().size()),
        std::vector<DataRow>(raw_uci.getTrainingData().size())
    };
    DataSet *uci_test = new DataSet {
        FeatureVectorHeader(raw_uci.getTestData()[0].x.size(), FeatureType::NUMERIC),
        (int)(raw_uci.getLabels().size()),
        std::vector<DataRow>(raw_uci.getTestData().size())
    };

    std::map<std::string, int> label_map;
    std::vector<std::string> labels;

    for(unsigned int i = 0; i < raw_uci.getTrainingData().size(); i++) {
        CSVRow temp = raw_uci.getTrainingData()[i];
        uci_training->rows[i].x = FeatureVector(temp.x.size());
        for(unsigned int j = 0; j < temp.x.size(); j++) {
            uci_training->rows[i].x[j] = temp.x[j];
        }
        if(label_map.find(temp.y) == label_map.end()) {
            int fresh = labels.size();
            labels.push_back(temp.y);
            label_map.emplace(temp.y, fresh);
        }
        uci_training->rows[i].y = label_map[temp.y];
    }
    // XXX copy-paste
    for(unsigned int i = 0; i < raw_uci.getTestData().size(); i++) {
        CSVRow temp = raw_uci.getTestData()[i];
        uci_test->rows[i].x = FeatureVector(temp.x.size());
        for(unsigned int j = 0; j < temp.x.size(); j++) {
            uci_test->rows[i].x[j] = temp.x[j];
        }
        if(label_map.find(temp.y) == label_map.end()) {
            int fresh = labels.size();
            labels.push_back(temp.y);
            label_map.emplace(temp.y, fresh);
        }
        uci_test->rows[i].y = label_map[temp.y];
    }

    ExperimentData *ret = new ExperimentData { uci_training, uci_test, labels };
    return ret;
}

const ExperimentData* ExperimentDataWrangler::fetch(const ExperimentDataEnum &dataset) {
    if(cache.find(dataset) != cache.cend()) {
        return cache.at(dataset);
    } else {
        loadData(dataset);
        return cache.at(dataset);
    }
}
