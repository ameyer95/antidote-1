#!/bin/bash

# copied and modified next_step.sh

# first argument is a memory limit (in MB, we convert to KB)
# second argument is the file with the list of commands to run
# third argument is the file where we will write these results
# fourth argument is the directory in which to store temp files (to avoid parallelized collisions)
MEMLIMIT=$[$1*2**10]
COMMANDSFILE=$2
OUTPUTFILE=$3
TEMPPATH=$4

RESULTFILE=$TEMPPATH/output.tmp
RESOURCEFILE=$TEMPPATH/resources.tmp

function run_benchmark {
    /usr/bin/time -f "%E %M" -o $RESOURCEFILE ./run_with_mem_limit.sh $MEMLIMIT $@ > $RESULTFILE
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
