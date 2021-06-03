# AntIDoTe-P

Using **A**bstract **I**nterpretation to reason about
**D**ecision-**T**ree learning's robustness to
training-set bias, **P**rogrammable version.

## Build Instructions

The top-level Makefile handles (nearly) everything; simply run `make`.
This creates the main binary, written to `bin/main`.
We recommend that you use a Linux system (there will be compiler issues on a Mac). 
Specifically, we wrote and ran the code on a machine that uses 18.04 LTS.

To run the experiments that use the [MNIST dataset](http://yann.lecun.com/exdb/mnist/),
the dataset must first be downloaded (and unzipped).
This is scripted: run `data/fetch-mnist.sh`.

Unit tests (the [test/](test/) directory) can be built and run using `make test`.
Testing uses the open source [Catch2](https://github.com/catchorg/Catch2) framework
whose license permits including its source in this repository
(as the single&mdash;but rather large&mdash;header file [test/catch2/](test/catch2/)catch.hpp).
Testing is currently very incomplete.

## Running tests

### Running a single test (non-targeted)
To run a single test, run
`bin/main -data data ` \< dataset name \> `-t ` \< test index \> `-d` \<depth\> `-V -l` \< l\> `-m` \< m\> `-n` \< k \>,  where l is the maximum number of labels to flip, m is the maximum number of missing samples to add, and k is the maximum number of fake samples to remove. If any of these variables are 0, you can simply omit that parameter. If all three are 0 (i.e., you are testing the baseline prediction), you can omit `-V` and everything that follows. 

For example, to test the first test sample of Adult Income with up to 20 flipped labels, we would execute:
`bin/main -data data adult_income -t 0 -d 2 -V -l 20`

To test the ninth test sample of Drug Consumption with up to 4 missing data points and up to 4 fake data points, we can execute:
`bin/main -data data drug_consumption -t 9 -d 1 -V -m 4 -n 4`

### Interpreting the output
The output is a JSON string containing information about the classifications. `Posterior` indicates the probability that the test sample belongs to each class (in terms of the paper, this output is pr^a). `Possible_classifications` will be a single value if the best-posterior class does not overlap with the posterior for any other class (in the binary case, if the two intervals do not overlap). When there is a single possible classification, the test sample is certifiably robust. Otherwise, there will be multiple possible classificaitons, which indicates that we cannot prove whether or not the test sample is robust.

E.g., 
`{ "depth" : 1, "test_index" : 0, "ground_truth" : "0", "posterior" : { "1" : [ 0.160338, 0.260532 ], "0" : [ 0.739468, 0.839662 ] }, "possible_classifications" : [ "0" ] }`
represents a test sample that is robust and that is assigned class 0 with a probability between 0.74 and 0.84.

### Running multiple tests


### Running targeted tests
Antidote-P is currently hard-coded to run targeted tests on any predicate that includes `label=positive`. (E.g., on the COMPAS dataset for race=Black and label=positive.) To change this to use label=positive (e.g., to replicate the Adult Income experiments on gender=Female and label=negative), there are several lines that need to be (un)commented in src/information_math.cpp/estimateCategorical. They all have inline-comments starting with "AI" or "COMPAS". 

To run a targeted label-flipping test, replace the `-l` flag with `-l1` and provide three arguments: l (the max. number of labels to flip), index (the column index of the sensitive attribute, e.g., race), and value (the target value of the sensitive attribute, e.g., Black). For example, in the COMPAS dataset `African_American` has column index 5 and a value of 1 if True, so we would run `bin/main -data data compas -t 0 -d 1 -V -l1 10 5 1` to test the robustness of COMPAS test sample 0 under ten label-flips of samples with race=Black and label=positive.

Targeted missing data can be performed in the same way, with the flag `m1`. The implementation does not support targeted fake data yet.
 
## Extending to other datasets

To use Antidote-P on other datasets, there are two options:
1. Update CommonEnums.h, CommonEnums.cpp, UCI.h, UCI.cpp, and ExperimentDataWrangler.cpp with the new dataset name and details. (Search the code for adult_income to find all places to update.) In UCI.h, you specify the name of the training data, test data,  the number of columns, the column-index of the label, any indices to ignore when creating the decision tree, and the total number of data (test+train). 
2. Use the ARFF format, place the data in the `data` folder, and pass in USE_ARFF as the dataset name (see https://www.cs.waikato.ac.nz/ml/weka/arff.html for a specification of this format). When using this option, there are some additional assumptions of the input data, mainly to support data labeling and boolean attributes: 
** If no `-i` argument (`label_index`) is specified, the first nominal data will be treated as label, while others are ignored 
** If `-i` argument is used, the parser will look for the attribute specified as label, and treat other binary nominal values as booleans, ignoring others.

 