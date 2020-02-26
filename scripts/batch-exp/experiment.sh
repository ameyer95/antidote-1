#!/bin/bash

# This is the top-level script for performing the incremental experiments
# in which we binary search for the maximal verifiable poisoning amount.
# It creates a sequence of files of the form "[N].jsonl";
# it must begin with a text file containing the initial commands to be run
# (e.g. the ones produced by ./initial_commands.sh)
# passed as an argument, and all the results files produced are stored in that same directory.
# There is no check for name collisions so be careful not to overwrite other experiments!

# first argument is the memory limit in MB
# second argument is the initcommands_* file, whose directory
# we will use to store all of the subsequent experiment .jsonl files
# (Note that we convert the argument to an absolute path.)
MEMLIMIT=$1
INITFILE=$(readlink -m $2)
PATHTOFILE=${INITFILE%\/*}

cd $(dirname "$0") # Descend into the same directory as the script

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

# Output a summary of the results afterwards
python3 ../data-wrangle/summarize.py $(ls $PATHTOFILE/*.jsonl) | column -t -s ,
