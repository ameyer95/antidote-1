#!/bin/bash

# XXX copied and modified next_step.sh
# to be able to run an array of experiments without performing the
# iterative experiment step preparations.

# first argument is a memory limit (in MB, we convert to KB)
# second argument is a time limit (in seconds)
# third argument is the file with the list of commands to run
# fourth argument is the file where we will write these results
# fifth argument is the directory in which to store temp files (to avoid parallelized collisions)
# (Note that we convert the path arguments to an absolute path.)
MEMLIMIT=$[$1*2**10]
TIMELIMIT=$2
COMMANDSFILE=$(readlink -m $3)
OUTPUTFILE=$(readlink -m $4)
TEMPPATH=$(readlnk -m $5)

RESULTFILE=$TEMPPATH/output.tmp
RESOURCEFILE=$TEMPPATH/resources.tmp

cd $(dirname "$0") # Descend into the same directory as the script

function run_benchmark {
    /usr/bin/time -f "%E %M" -o $RESOURCEFILE ./run_with_mem_limit.sh $MEMLIMIT $TIMELIMIT $@ > $RESULTFILE
    if [ $? -eq 0 ]
    then
        RESULT=$(cat $RESULTFILE)
        NORMALEXIT=true
    else
        RESULT=null
        NORMALEXIT=false
    fi
    # The time -o flag doesn't stop additional stderr from time to be written to the file,
    # so we take the last line before passing to awk.
    TIME=$(tail -n 1 $RESOURCEFILE | awk '{print $1}')
    # Note that 1) the mem limit script polls, so max memory can exceed the limit
    # and 2) if the mem limit script kills the process, the reported memory is incorrect
    MEMORY=$(tail -n 1 $RESOURCEFILE | awk '{print $2}')
    echo "{ \"command\" : \"$@\", \"normal_exit\" : $NORMALEXIT, \"result\" : $RESULT, \"time\" : \"$TIME\", \"memory\" : \"$MEMORY\" }" >> $OUTPUTFILE
    rm $RESULTFILE $RESOURCEFILE
}

INDEX=1
TOTAL=$(echo $(wc -l $COMMANDSFILE) | awk '{print $1}')
while read C ; do
    echo $(date) running "("$INDEX/$TOTAL")" $C
    run_benchmark $C
    INDEX=$[$INDEX + 1]
done < $COMMANDSFILE
