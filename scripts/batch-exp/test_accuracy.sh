cd $(dirname "$0")

OUTDIR=../../concrete

# Make the directory in which we store results (if it doesn't exist)
mkdir -p $OUTDIR

for DATASET in iris mammography cancer mnist_simple_1_7 mnist_1_7; do
    for DEPTH in {1..4}; do
        OUTFILE=$OUTDIR/test_${DATASET}_d${DEPTH}.jsonl
        COMMAND="../../bin/main -f ../../data $DATASET -d $DEPTH -T"
        echo -n $(date) dataset=$DATASET depth=$DEPTH ... \ 
        $COMMAND > $OUTFILE
        echo accuracy=$(python3 ../data-wrangle/accuracy.py $OUTFILE)
    done
done
