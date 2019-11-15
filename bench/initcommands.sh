#!/bin/bash

mkdir iris
for D in {1..3}
do
    mkdir iris/d${D}_a iris/d${D}_V
    for I in {0..29}
    do
        echo ../bin/main -f ../data iris -d $D -t $I -a 0 >> iris/d${D}_a/initcommands_iris_d${D}_a.txt
        echo ../bin/main -f ../data iris -d $D -t $I -V 0 >> iris/d${D}_V/initcommands_iris_d${D}_V.txt
    done
done

mkdir mammography
for D in {1..4}
do
    mkdir mammography/d${D}_a mammography/d${D}_V
    for I in {0..165}
    do
        echo ../bin/main -f ../data mammography -d $D -t $I -a 0 >> mammography/d${D}_a/initcommands_mammography_d${D}_a.txt
        echo ../bin/main -f ../data mammography -d $D -t $I -V 0 >> mammography/d${D}_V/initcommands_mammography_d${D}_V.txt
    done
done

mkdir cancer
for D in {1..6}
do
    mkdir cancer/d${D}_a cancer/d${D}_V
    for I in {0..112}
    do
        echo ../bin/main -f ../data cancer -d $D -t $I -a 0 >> cancer/d${D}_a/initcommands_cancer_d${D}_a.txt
        echo ../bin/main -f ../data cancer -d $D -t $I -V 0 >> cancer/d${D}_V/initcommands_cancer_d${D}_V.txt
    done
done
