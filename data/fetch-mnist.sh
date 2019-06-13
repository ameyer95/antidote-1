#!/bin/bash

URLBASE=yann.lecun.com/exdb/mnist/
FILES=(train-images-idx3-ubyte.gz train-labels-idx1-ubyte.gz t10k-images-idx3-ubyte.gz t10k-labels-idx1-ubyte.gz)
URLS=($(for FILENAME in ${FILES[@]}; do echo "${URLBASE}${FILENAME}"; done))

cd $(dirname "$0") # Descend into the same directory as the script
wget ${URLS[@]}
gunzip ${FILES[@]}
