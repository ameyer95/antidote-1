#!/bin/bash

# first argument is the memory limit in MB
# second argument is the initcommands_* file, whose directory
# we will use to store all of the subsequent experiment .jsonl files
DATA=$1
DEPTH=$2
L=$3
M=$4
N=$5
START=$6
NUM=$7
END=$(($NUM+$START-1))

for I in $(seq $START $END)
do
    bin/main -data data $DATA -t $I -d $DEPTH -V -l $L -m $M -n $N
done