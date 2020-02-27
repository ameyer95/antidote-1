# Builds the ./run_all.sh script to recreate all the experiments.
# (Deletes an existing one, if present)

cd $(dirname "$0") # Descend into the same directory as the script

OUTFILE=run_all.sh

rm -f $OUTFILE
echo \# Two arguments: MEMLIMIT and TIMELIMIT, matching experiment.sh > $OUTFILE
echo MEMLIMIT=\$1 >> $OUTFILE
echo TIMELIMIT=\$2 >> $OUTFILE
echo "cd \$(dirname \"\$0\") # Descend into the same directory as this script" >> $OUTFILE

for DATASET in iris mammography cancer mnist_simple_1_7 mnist_1_7; do
    for DOMAIN in a V; do
        for DEPTH in {1..4}; do
            echo ./experiment.sh \$MEMLIMIT \$TIMELIMIT ../../bench/$DATASET/d${DEPTH}_${DOMAIN}/initcommands_${DATASET}_d${DEPTH}_${DOMAIN}.txt >> $OUTFILE
        done
    done
done

chmod +x $OUTFILE
echo Written to $(readlink -m $OUTFILE)
