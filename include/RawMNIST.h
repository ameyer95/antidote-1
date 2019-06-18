#ifndef RAWMNIST_H
#define RAWMNIST_H

#include "RawData.h"
#include "MNISTReader.h"
#include <cstdint>
#include <utility>
#include <string>
using namespace std;

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

#endif
