#!/bin/bash

# Runs a command, and while waiting for it to finish,
# periodically checks if the process is exceeding a given memory threshold.
# If so, kills the process and exits with an error.

# memory limit in KB
MEMLIMIT=$1
# All tokens after the first argument constitute the single command
COMMAND=${@:2}

# Also, we give all benchmarks a 1 hour time limit
TIMELIMIT=3600

$COMMAND &
PID=$!
while true
do
    # memory in KB
    MEMORY=$(ps -p $PID --no-headers --format rss)
    TIME=$(ps -p $PID --no-headers --format etime)
    TIME=$(./scripts/chtc/hhmmss.sh $TIME)
    if [ -z "$MEMORY" ]
    then
        # process must have exited normally
        exit 0
    elif [ $MEMORY -gt $MEMLIMIT ]
    then
        # process needs to be killed
        kill -9 $PID
        exit 1
    elif [ $TIME -gt $TIMELIMIT ]
    then
        # process needs to be killed
        kill -9 $PID
        exit 1
    else
        # wait .1 second before checking again
        sleep .1
    fi
done
