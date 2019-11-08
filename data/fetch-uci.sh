#!/bin/bash

URLBASE=archive.ics.uci.edu/ml/machine-learning-databases/

declare -A MAPBASETOFILES
MAPBASETOFILES["iris"]="iris.names bezdekIris.data"
MAPBASETOFILES["breast-cancer-wisconsin"]="wdbc.data wdbc.names"
MAPBASETOFILES["wine"]="wine.data wine.names"

URLS=($(
for KEY in ${!MAPBASETOFILES[@]}; do
    for FILE in ${MAPBASETOFILES[$KEY]}; do
        echo ${URLBASE}${KEY}/${FILE};
    done;
done;))

cd $(dirname "$0") # Descend into the same directory as the script
wget -N ${URLS[@]}

python3 train_test_split.py
