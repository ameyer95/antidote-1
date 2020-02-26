#!/bin/bash

# Takes one argument: a string of time as produced by $(ps -p $PID --no-headers --format etime)
# and converts it into seconds.
# This is an auxiliary method used by ./run_with_mem_limit.sh

HHMMSS=$1
IFS=:
TOTAL=0
for N in ${HHMMSS%.*}
do
    TOTAL=$((60*10#$TOTAL))
    TOTAL=$((10#$TOTAL + 10#$N))
done
echo $TOTAL
