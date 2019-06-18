#include "RawMNIST.h"
#include "MNISTReader.h"
#include <cstdint>
#include <utility>
#include <string>
using namespace std;

RawMNIST::RawMNIST(MNISTMode mode, const string prefix) {
    pair<LabelFile, ImageFile> files;
    if(mode == MNISTMode::TRAINING) {
        files = MNIST_readTrainingSet(prefix);
    } else {
        files = MNIST_readTestSet(prefix);
    }
    image_file = files.second;
    label_file = files.first;
}

const Image RawMNIST::getImage(unsigned int i) const {
    int offset = i * image_file.num_rows * image_file.num_columns * sizeof(uint8_t);
    return image_file.pixels + offset;
}

const Label RawMNIST::getLabel(unsigned int i) const {
    int offset = i * label_file.num_items * sizeof(uint8_t);
    return *(label_file.labels + offset);
}

const pair<const Image, const Label> RawMNIST::operator [](unsigned int i) const {
    return make_pair(getImage(i), getLabel(i));
};

unsigned int RawMNIST::size() const {
    return image_file.num_items;
};
