#include "MNISTReader.h"
#include <cstdint>
#include <fstream>
#include <utility>
#include <cstdlib> // For exit, EXIT_FAILURE
#include <iostream>
using namespace std;

void outputAndQuit(const string message) {
    cout << message << endl;
    exit(EXIT_FAILURE);
}

void assertGoodRead(ifstream &file, const string filename="") {
    if(file.fail()) {
        outputAndQuit("Error reading file " + filename);
    }
}

// In the MNIST dataset, the 4-byte integers are stored big-endian, not little-endian
void endianSwap(int32_t &x) {
    uint8_t *c = (uint8_t*)&x;
    swap(*c, *(c+3));
    swap(*(c+1), *(c+2));
}

void read32Field(ifstream &file, int32_t &field, const string filename="") {
    file.read((char*)&field, sizeof(field));
    assertGoodRead(file, filename);
    endianSwap(field);
}

void readBytesField(ifstream &file, uint8_t *&field, int num_items, const string filename="") {
    field = new uint8_t[num_items];
    file.read((char*)field, sizeof(uint8_t) * num_items);
    assertGoodRead(file, filename);
}

LabelFile readLabelFile(const string full_path) {
    LabelFile ret;
    ifstream file(full_path, ios::binary | ios::in);

    read32Field(file, ret.magic_number, full_path);
    if(ret.magic_number != MNIST_LABEL_MAGIC_NUMBER) {
        outputAndQuit("Unexpected magic number in " + full_path);
    }
    read32Field(file, ret.num_items, full_path);
    readBytesField(file, ret.labels, ret.num_items, full_path);

    file.close();
    return ret;
}

ImageFile readImageFile(const string full_path) {
    ImageFile ret;
    ifstream file(full_path, ios::binary | ios::in);

    read32Field(file, ret.magic_number, full_path);
    if(ret.magic_number != MNIST_IMAGE_MAGIC_NUMBER) {
        outputAndQuit("Unexpected magic number in " + full_path);
    }
    read32Field(file, ret.num_items, full_path);
    read32Field(file, ret.num_rows, full_path);
    read32Field(file, ret.num_columns, full_path);
    readBytesField(file, ret.pixels, ret.num_items * ret.num_rows * ret.num_columns, full_path);

    file.close();
    return ret;
}

pair<LabelFile, ImageFile> MNIST_readTrainingSet(const string prefix) {
    return make_pair(readLabelFile(prefix + MNIST_TRAINING_LABEL_FILE),
                     readImageFile(prefix + MNIST_TRAINING_IMAGE_FILE));
}

pair<LabelFile, ImageFile> MNIST_readTestSet(const string prefix) {
    return make_pair(readLabelFile(prefix + MNIST_TEST_LABEL_FILE),
                     readImageFile(prefix + MNIST_TEST_IMAGE_FILE));
}
