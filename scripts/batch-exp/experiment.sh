#!/bin/bash

# first argument is the memory limit in MB
# second argument is the initcommands_* file, whose directory
# we will use to store all of the subsequent experiment .jsonl files
MEMLIMIT=$1
INITFILE=$2
PATHTOFILE=${INITFILE%\/*}

NEXT=1
./next_step.sh $MEMLIMIT $INITFILE $PATHTOFILE/$NEXT.jsonl $PATHTOFILE
while true
do
    PREV=$NEXT
    NEXT=$[$NEXT + 1]
    PREVFILE=$PATHTOFILE/$PREV.jsonl
    NEXTFILE=$PATHTOFILE/$NEXT.jsonl
    # check if the prevous file exists and is non-empty,
    # i.e. check if the previous iteration actually caused us to run any experiments
    if [ -s $PREVFILE ]
    then
        ./next_step.sh $MEMLIMIT $PREVFILE $NEXTFILE $PATHTOFILE
    else
        break
    fi
done

python3 summarize.py $(ls $PATHTOFILE/*.jsonl) | column -t -s ,
