#ifndef MNIST_H
#define MNIST_H

#include "RawData.h"
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

const int32_t MNIST_IMAGE_SCALE = 28;
const int32_t MNIST_IMAGE_SIZE = MNIST_IMAGE_SCALE * MNIST_IMAGE_SCALE;

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

enum class MNISTMode { TRAINING, TEST };

typedef uint8_t* Image;
typedef uint8_t Label;


class RawMNIST : public RawData<Image,Label> {
private:
    ImageFile image_file;
    LabelFile label_file;

public:
    RawMNIST(MNISTMode mode, const string prefix);
    const Image getImage(unsigned int i) const;
    const Label getLabel(unsigned int i) const;
    const pair<const Image, const Label> operator [](unsigned int i) const;
    unsigned int size() const;
};


pair<LabelFile, ImageFile> MNIST_readTrainingSet(const string prefix);
pair<LabelFile, ImageFile> MNIST_readTestSet(const string prefix);

#endif
