#!/bin/bash

# Runs a command, and while waiting for it to finish,
# periodically checks if the process is exceeding a given memory threshold.
# If so, kills the process and exits with an error.
# (Update: now also enforces a given timeout.)
#
# Ideally, we wouldn't need this auxiliary script,
# since it is supposed to be achievable with ulimit,
# but it appears to be known that this functionality is broken
# in recent versions of the linux kernel...
# Instead, we run the command in the background and iteratively poll its usage.

# memory limit in KB
MEMLIMIT=$1
# time limit in seconds (used to be hard-coded 1hr=3600)
TIMELIMIT=$2
# All tokens after the second argument constitute the single command
COMMAND=${@:3}

# Don't descend into this directory (in case command includes relative paths)
# but access ./hhmmss.sh using relative paths
THISDIR=$(dirname $0)

$COMMAND &
PID=$!
while true
do
    # memory in KB
    MEMORY=$(ps -p $PID --no-headers --format rss)
    TIME=$(ps -p $PID --no-headers --format etime)
    TIME=$($THISDIR/hhmmss.sh $TIME)
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
