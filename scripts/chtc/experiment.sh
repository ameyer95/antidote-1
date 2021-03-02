#!/bin/bash

# first argument is the memory limit in MB
# second argument is the name of the dataset (e.g., iris or MNIST_1_7)
# third argument is depth
# fourth argument is the poisoning level
# fifth element is the starting index of the dataset
# sixth element is the number of elements to test in one job
MEMLIMIT=$1
DATASET=$2
DEPTH=$3
POISONING=$4
START=$5
NUM_TO_RUN=$6
END=$(($NUM_TO_RUN+$START-1))

#mkdir scripts/chtc/$DATASET
#mkdir scripts/chtc/${DATASET}/d${DEPTH}_V${POISONING}
#INITFILE=scripts/chtc/${DATASET}/d${DEPTH}_V${POISONING}/output.txt
#PATHTOFILE=${INITFILE%\/*}

cd antidote

# Store the commands in a text file
for I in $(seq $START $END)
do
    bin/main -f data $DATASET -d $DEPTH -t $I #-V $POISONING -l
done

#cat $INITFILE

#python3 scripts/chtc/summarize.py $(ls ${PATHTOFILE}/*.jsonl) | column -t -s , >> summary.txt
