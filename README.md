# AntIDoTe-P

Using **A**bstract **I**nterpretation to reason about
**D**ecision-**T**ree learning's robustness to
training-set bias, **P**rogrammable version.

## Build Instructions

The top-level Makefile handles (nearly) everything; simply run `make`.
This creates the main binary, written to `bin/main`.

To run the experiments that use the [MNIST dataset](http://yann.lecun.com/exdb/mnist/),
the dataset must first be downloaded (and unzipped).
This is scripted: run `data/fetch-mnist.sh`.

Unit tests (the [test/](test/) directory) can be built and run using `make test`.
Testing uses the open source [Catch2](https://github.com/catchorg/Catch2) framework
whose license permits including its source in this repository
(as the single&mdash;but rather large&mdash;header file [test/catch2/](test/catch2/)catch.hpp).
Testing is currently very incomplete.

## Mathematical Formalization

The latex document in [doc/](doc/) details the specification of the
concrete and abstract semantics implemented by the code base;
running `pdflatex paper` within the directory will generate the pdf.

## File format support 

We use ARFF format for user data. A general specification of the format is [here](https://www.cs.waikato.ac.nz/ml/weka/arff.html). 

There are some additional assumptions of the input data, mainly to support data labeling and boolean attributes: 
* If no `-i` argument (`label_index`) is specified, the first nominal data will be treated as label, while others are ignored 
* If `-i` argument is used, the parser will look for the attribute specified as label, and treat other binary nominal values as booleans, ignoring others. 

For an example arff dataset with the data of the "Illustrative example" from the paper (Fig. 2), see `data/example.arff`. 
