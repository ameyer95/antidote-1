#!/bin/bash
HHMMSS=$1
IFS=:
TOTAL=0
for N in ${HHMMSS%.*}
do
    TOTAL=$((60*10#$TOTAL))
    TOTAL=$((10#$TOTAL + 10#$N))
done
echo $TOTAL
