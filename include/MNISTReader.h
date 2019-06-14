#ifndef MNISTREADER_H
#define MNISTREADER_H

#include "DataSet.h"
#include <cstdint>
#include <utility>
#include <string>
using namespace std;

const string MNIST_TRAINING_LABEL_FILE = "train-labels-idx1-ubyte";
const string MNIST_TRAINING_IMAGE_FILE = "train-images-idx3-ubyte";
const string MNIST_TEST_LABEL_FILE = "t10k-labels-idx1-ubyte";
const string MNIST_TEST_IMAGE_FILE = "t10k-images-idx3-ubyte";

const int32_t MNIST_LABEL_MAGIC_NUMBER = 0x00000801;
const int32_t MNIST_IMAGE_MAGIC_NUMBER = 0x00000803;

struct LabelFile {
    int32_t magic_number;
    int32_t num_items;
    uint8_t *labels;
};

struct ImageFile {
    int32_t magic_number;
    int32_t num_items;
    int32_t num_rows;
    int32_t num_columns;
    uint8_t *pixels;
};

pair<LabelFile, ImageFile> MNIST_readTrainingSet(const string prefix);
pair<LabelFile, ImageFile> MNIST_readTestSet(const string prefix);

pair<DataSet*, DataSet*> MNIST_readAsDataSet(const string prefix);

#endif
