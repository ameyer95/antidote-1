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

mkdir scripts/chtc/$DATASET
mkdir scripts/chtc/${DATASET}/d${DEPTH}_V${POISONING}_labels
INITFILE=scripts/chtc/${DATASET}/d${DEPTH}_V${POISONING}_labels/initcommands_${DATASET}_d${DEPTH}_V${POISONING}_labels.txt
PATHTOFILE=${INITFILE%\/*}

# Store the commands in a text file
for I in $(seq $START $END)
do
    echo bin/main -f data $DATASET -d $DEPTH -t $I -V $POISONING -l >> $INITFILE
done

NEXT=1
echo "PATH TO FILE (THIS WILL BE TEMPFILE) IS "$PATHTOFILE
./scripts/chtc/next_step_chtc.sh $MEMLIMIT $INITFILE $PATHTOFILE/$NEXT.jsonl $PATHTOFILE 
echo "FINISHED FIRST NEXT STEP CALL"
while true
do
    PREV=$NEXT
    NEXT=$[$NEXT + 1]
    PREVFILE=$PATHTOFILE/$PREV.jsonl
    NEXTFILE=$PATHTOFILE/$NEXT.jsonl
    # check if the prevous file exists and is non-empty,
    # i.e. check if the previous iteration actually caused us to run any experiments
    echo "Path to file is "$PATHTOFILE" and prevFile is "$PREVFILE" and nextFile is "$NEXTFILE
    if [ -s $PREVFILE ]
    then
        ./scripts/chtc/next_step_chtc.sh $MEMLIMIT $PREVFILE $NEXTFILE $PATHTOFILE
    else
        break
    fi
done

python3 scripts/chtc/summarize.py $(ls ${PATHTOFILE}/*.jsonl) | column -t -s ,
