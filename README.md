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
This is scripted: run `data/fetch-mnist.sh`. The data used for other experiments in our paper
(namely, COMPAS, Adult Income, and Drug Consumption) are in the Data folder (each dataset has a 
train and a test file).

Unit tests (the [test/](test/) directory) can be built and run using `make test`.
Testing uses the open source [Catch2](https://github.com/catchorg/Catch2) framework
whose license permits including its source in this repository
(as the single&mdash;but rather large&mdash;header file [test/catch2/](test/catch2/)catch.hpp).
Testing is currently *very* incomplete.

## Running tests

### Running a single test (non-targeted)
To run a single test, run
`bin/main -data data ` \< dataset name \> `-t ` \< test index \> `-d` \<depth\> `-V -l` \< l\> `-m` \< m\> `-n` \< k \>,  where l is the maximum number of labels to flip, m is the maximum number of missing samples to add, and k is the maximum number of fake samples to remove. If any of these variables are 0, you can simply omit that parameter. If all three are 0 (i.e., you are testing the baseline prediction), you can omit `-V` and everything that follows. 

For example, to test the first test sample of Adult Income with up to 20 flipped labels, we would execute:
`bin/main -data data adult_income -t 0 -d 2 -V -l 20`

To test the ninth test sample of Drug Consumption with up to 4 missing data points and up to 4 fake data points, we can execute:
`bin/main -data data drug_consumption -t 9 -d 1 -V -m 4 -n 4`

### Interpreting the output
The output is a JSON string containing information about the classifications. `Posterior` indicates the probability that the test sample belongs to each class (in terms of the paper, this output is $pr^a$). `Possible_classifications` will be a single value if the best-posterior class does not overlap with the posterior for any other class (in the binary case, if the two intervals do not overlap). When there is a single possible classification, the test sample is certifiably robust. Otherwise, there will be multiple possible classificaitons, which indicates that we cannot prove whether or not the test sample is robust.

E.g., 
`{ "depth" : 1, "test_index" : 0, "ground_truth" : "0", "posterior" : { "1" : [ 0.160338, 0.260532 ], "0" : [ 0.739468, 0.839662 ] }, "possible_classifications" : [ "0" ] }`
represents a test sample that is robust and that is assigned class 0 with a probability between 0.74 and 0.84.

### Running multiple tests
To run the same test on multiple test samples (e.g., testing robustness at 0.5% label flipping on all test data), use scripts/experiment.sh, which takes command-line arguments of dataset, depth, l, m, n, start, and number to run.
For example, `./scripts/experiment.sh compas 1 8 0 0 0 100 >> test.json` will test robustness against 8 label-flips of the first 100 elements of the COMPAS dataset, and then save the results in a file called test.json. Running this particular test should take fewer than 5 seconds, but testing additional test samples, a higher poisoning threshold, or more time-intensive datasets will take longer. If in doubt, run a single test first to get a sense of the expected time!

To analyze the results of the json file, use scripts/analyze-single-json.py, which takes two parameters: filename, and mnist (1 if running MNIST, 0 for any other dataset). For example, to see the certifiably-robust percentage of the above command, we would run `python3 scripts/analyze-single-json.py scripts/test.json 0`. In this case, the output is 48%. 

### Running targeted tests
Antidote-P is currently hard-coded to run targeted tests on any predicate that includes `label=positive`. (E.g., on the COMPAS dataset for race=Black and label=positive.) To change this to use label=positive (e.g., to replicate the Adult Income experiments on gender=Female and label=negative), there are several lines that need to be (un)commented in src/information_math.cpp/estimateCategorical. They all have inline-comments starting with "AI" or "COMPAS". 

For Adult Income, index 8 is race, which has been preprocessed to be binary with white=0 and non-white=1, and index 9 is sex with 0=male and 1=female. For COMPAS, index 5 is the one-hot encoded columns for Race=African-American, index 10 is sex with 1=Female and 0=Male.

To run a targeted label-flipping test, replace the `-l` flag with `-l1` and provide three arguments: l (the max. number of labels to flip), index (the column index of the sensitive attribute, e.g., race), and value (the target value of the sensitive attribute, e.g., Black). For example, to test the robustness of COMPAS test sample 0 under ten label-flips with the predicate g := (race=Black and label=1), we would execute 

`bin/main -data data compas -t 0 -d 1 -V -l1 10 5 1` 

Targeted missing data can be performed in the same way, with the flag `m1`. The implementation does not support targeted fake data yet.
 
## Data

### How was the data preprocessed?
Details about preprocessing is available in Jupyter notebooks in the scripts/ folder. (With the exception of MNIST -- all preprocessing for this file happens in src/ExperimentDataWrangler.cpp.) Preprocessing mainly consisted of converting string-based categorical variables to integers. 

We used standard train/test splits when they were available, otherwise, we used our own custom splits. The data (processed and divided into train and test sets) is available in the Data folder.

### Extending to other datasets
To use Antidote-P on other datasets, there are two options:
1. Update CommonEnums.h, CommonEnums.cpp, UCI.h, UCI.cpp, and ExperimentDataWrangler.cpp with the new dataset name and details. (Search the code for adult_income to find all places to update.) In UCI.h, you specify the name of the training data, test data,  the number of columns, the column-index of the label, any indices to ignore when creating the decision tree, and the total number of data (test+train). 
2. Use the ARFF format, place the data in the `data` folder, and pass in USE_ARFF as the dataset name (see https://www.cs.waikato.ac.nz/ml/weka/arff.html for a specification of this format). When using this option, there are some additional assumptions of the input data, mainly to support data labeling and boolean attributes: 
** If no `-i` argument (`label_index`) is specified, the first nominal data will be treated as label, while others are ignored 
** If `-i` argument is used, the parser will look for the attribute specified as label, and treat other binary nominal values as booleans, ignoring others.

## Questions?
Please let me know if you have questions or would appreciate a walkthrough of the code. I will do my best to respond & help!
 
