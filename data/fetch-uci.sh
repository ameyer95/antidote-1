#!/bin/bash

URLBASE=archive.ics.uci.edu/ml/machine-learning-databases/

declare -A MAPBASETOFILES
MAPBASETOFILES["iris"]="iris.names bezdekIris.data"
MAPBASETOFILES["breast-cancer-wisconsin"]="wdbc.data wdbc.names"
MAPBASETOFILES["wine-quality"]="winequality-red.csv winequality-white.csv winequality.names"
MAPBASETOFILES["yeast"]="yeast.data yeast.names"
MAPBASETOFILES["00329"]="messidor_features.arff"

URLS=($(
for KEY in ${!MAPBASETOFILES[@]}; do
    for FILE in ${MAPBASETOFILES[$KEY]}; do
        echo ${URLBASE}${KEY}/${FILE};
    done;
done;))

cd $(dirname "$0") # Descend into the same directory as the script
wget -N ${URLS[@]}

# wine quality dataset special handling
tail -n +2 winequality-red.csv > winequality.data
tail -n +2 winequality-white.csv >> winequality.data
sed -i 's/;/,/g' winequality.data

# yeast dataset special handling
sed -i 's/\s\s*/,/g' yeast.data

# retinopathy dataset special handling
tail -n +25 messidor_features.arff > retinopathy.data

python3 train_test_split.py
