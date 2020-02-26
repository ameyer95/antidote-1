#!/bin/bash

# Creates the initial_commands* files needed by the iterative ./experiment.sh script.
# Please only call this once: it always makes the [top-level]/bench directory
# and then assumes it is freshly creating all contents of the form
# bench/{iris, mammography, cancer, mnist_simple_1_7, mnist_1_7}/d*_{a, V}/*
#
# All the commands it creates in the initial_commands* files
# assume those commands are being run from the scripts/batch-exp directory
# (as is the case, since the ./run_with_mem_limit.sh file
# that actually runs the commands is called from ./next_step.sh
# which appropriately descends into its directory).

cd $(dirname "$0") # Descend into the same directory as the script

# Make the benchmark directory if it does not exist
BENCHDIR=../../bench
mkdir -p $BENCHDIR

mkdir $BENCHDIR/iris
for D in {1..4}
do
    mkdir $BENCHDIR/iris/d${D}_a $BENCHDIR/iris/d${D}_V
    for I in {0..29}
    do
        echo ../../bin/main -f ../../data iris -d $D -t $I -a 0 >> $BENCHDIR/iris/d${D}_a/initcommands_iris_d${D}_a.txt
        echo ../../bin/main -f ../../data iris -d $D -t $I -V 0 >> $BENCHDIR/iris/d${D}_V/initcommands_iris_d${D}_V.txt
    done
done

mkdir $BENCHDIR/mammography
for D in {1..4}
do
    mkdir $BENCHDIR/mammography/d${D}_a $BENCHDIR/mammography/d${D}_V
    for I in {0..165}
    do
        echo ../../bin/main -f ../../data mammography -d $D -t $I -a 0 >> $BENCHDIR/mammography/d${D}_a/initcommands_mammography_d${D}_a.txt
        echo ../../bin/main -f ../../data mammography -d $D -t $I -V 0 >> $BENCHDIR/mammography/d${D}_V/initcommands_mammography_d${D}_V.txt
    done
done

mkdir $BENCHDIR/cancer
for D in {1..4}
do
    mkdir $BENCHDIR/cancer/d${D}_a $BENCHDIR/cancer/d${D}_V
    for I in {0..112}
    do
        echo ../../bin/main -f ../../data cancer -d $D -t $I -a 0 >> $BENCHDIR/cancer/d${D}_a/initcommands_cancer_d${D}_a.txt
        echo ../../bin/main -f ../../data cancer -d $D -t $I -V 0 >> $BENCHDIR/cancer/d${D}_V/initcommands_cancer_d${D}_V.txt
    done
done

mkdir $BENCHDIR/mnist_simple_1_7
for D in {1..4}
do
    mkdir $BENCHDIR/mnist_simple_1_7/d${D}_a $BENCHDIR/mnist_simple_1_7/d${D}_V
    for I in $(python3 test_mnist.py)
    do
        echo ../../bin/main -f ../../data mnist_simple_1_7 -d $D -t $I -a 0 >> $BENCHDIR/mnist_simple_1_7/d${D}_a/initcommands_mnist_simple_1_7_d${D}_a.txt
        echo ../../bin/main -f ../../data mnist_simple_1_7 -d $D -t $I -V 0 >> $BENCHDIR/mnist_simple_1_7/d${D}_V/initcommands_mnist_simple_1_7_d${D}_V.txt
    done
done

mkdir $BENCHDIR/mnist_1_7
for D in {1..4}
do
    mkdir $BENCHDIR/mnist_1_7/d${D}_a $BENCHDIR/mnist_1_7/d${D}_V
    for I in $(python3 test_mnist.py)
    do
        echo ../../bin/main -f ../../data mnist_1_7 -d $D -t $I -a 0 >> $BENCHDIR/mnist_1_7/d${D}_a/initcommands_mnist_1_7_d${D}_a.txt
        echo ../../bin/main -f ../../data mnist_1_7 -d $D -t $I -V 0 >> $BENCHDIR/mnist_1_7/d${D}_V/initcommands_mnist_1_7_d${D}_V.txt
    done
done
