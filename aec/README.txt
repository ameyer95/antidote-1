# 1 Getting Started Guide

The artifact is a docker image based on ubuntu:18.04,
and our testing of the artifact was done on a ubuntu:18.04 host machine
using Docker version 18.09.9, build 1752eb3.
Docker images are portable and run on a variety of operating systems,
but the instructions given in this guide will assume the user is operating
in a bash shell on a unix-like system.
All you should need to begin is the given antidote-image.tar file and a working
docker installation.


### 1.1 Setting up Docker

In a perfect world, the reader of this document would already be familiar with
docker and have it installed.
If this is the case, skip ahead; if not, please consult the following:

* <https://docs.docker.com/get-started/> for an explanation of images and
  containers, and for installation instructions for Windows and Mac.
* <https://docs.docker.com/install/linux/docker-ce/ubuntu/> for installation
  instructions on Ubuntu. Note, however, that the standard apt repositories have
  a docker version available: `sudo apt-get install docker.io`.
  Note, also, that non-root users must be given permissions to use docker:
  <https://docs.docker.com/install/linux/linux-postinstall/#manage-docker-as-a-non-root-user>.

We will provide instructions so that even an inexperienced docker user should be
able to run the given commands and recreate the desired results.
(At the end of this section, we also provide some notes about the usage and
maintenance of docker images and containers.)


### 1.2 Running the Image

Load the image from the .tar file into docker's internal storage
`docker load -i antidote-image.tar`
This should end with a successful "Loaded image: antidote:ae" message.
You can verify that the image appears in the output of `docker images`,
where it should have a reported size of approximately 160MB.

Start up an interactive shell in a container from the image:
`docker run -it --rm antidote:ae`
This should give you a bash shell prompt of "root@HASH:/antidote#".
Everything needed is contained in the top-level `/antidote` directory.

At any time you may exit the interactive docker container by using `exit`,
but because we used the `--rm` flag in the run command,
the container automatically deletes itself upon exiting.
If you do `exit`, you must perform the prior `docker run ...` command before
proceeding to the next section.


### 1.3 Running the Tool

From the shell within the interactive docker container,
you should be able to see the contents of /antidote with `ls`.
The tool comes pre-built (./bin/main) from the source (./include/ and ./src/).
Running `bin/main` without any arguments should simply output usage information
as a list of the form "-flag[number of arguments]: description".

The datasets from our experimental evaluation (and more) are stored in ./data/.
We will illustrate example usage of the tool with the simplest dataset: iris.
The following command trains a depth-2 decision tree on the iris training set
(data/iris80.data) and uses it to classify the test set element with index 0
(i.e. the first row of the test set file data/iris20.data):
`bin/main -f data iris -d 2 -t 0 | jq`
(The pipe into `jq` is optional, but makes the output more readable.)
The program should terminate immediately, outputting the following json string:

    {
      "depth": 2,
      "test_index": 0,
      "ground_truth": "Iris-setosa",
      "posterior": {
        "Iris-setosa": 1,
        "Iris-versicolor": 0,
        "Iris-virginica": 0
      },
      "possible_classifications": [
        "Iris-setosa"
      ]
    }

This output repeats some of the input parameters (depth and test index),
but also reports that

1. this test set instance's true label ("ground_truth") is "Iris-setosa";
2. the posterior distribution the model assigns this test input for its
   classification ("posterior") has probability 1 for "Iris-setosa";
3. accordingly, the tree (correctly) classifies the test input as an
   "Iris-setosa" (since "possible_classifications" is a single value).

This should exhibit that the main executable is runnable;
we will defer discussing other variations of parameters to the more detailed
Step-by-Step Instructions in the latter part of this document.


### 1.4 Running Batches of Experiments

As described in our paper, our tool uses abstract interpretation to prove an
adversary can not successfully perform a *training set poisoning attack*.
Without going into to much detail, the adversary is allowed to contribute N-many
poisoned training points, and we (try to) prove that all such training sets
yield models that classify a particular test input in the same way.

We have scripts that automate a process where we explore our ability to prove
poisoning robustness for increasing amounts of possible poisoning (this N).
(The full description is in the paper, Section 6.1, "Experimental Setup".)
We exemplify their usage, continuing with the depth-2 tree on the iris dataset.

There is a directory hierarchy in ./bench for batch experimentation. Run
`cat bench/iris/d2_V/initcommands_iris_d2_V.txt`
and observe that it simply contains a list of commands that invoke our tool,
one for each of the 30 elements in the test set.
The "-V 0" flag tells the tool to compute the adversarial possibilities for N=0,
an initial trivial case.
We have a pipeline that (1) runs each of these commands, and then
(2) for each input where we are able to prove the poisoning-robustness property,
we increase the value of N and run again, repeating until all proofs fail.

Run the following:
`scripts/batch-exp/experiment.sh 1000 60 bench/iris/d2_V/initcommands_iris_d2_V.txt`
(note that the first two arguments are a memory limit in MB and a time limit in
seconds, respectively, for each of the commands that are run).
It should terminate within a minute, outputting timestamps for each of the
individual commands it runs.
It then outputs a summary of the results in a tabular format, as below
(omitting/altering some columns, and resource information may vary slightly):

    num_dropout  num_run  num_verified  average_time  average_memory
    0            30       26            0.14          3792
    1            26       22            0.15          3844
    2            22       19            0.15          4024
    3            19       8             0.15          4243
    4            19       0             0.15          4538

The fourth data row reads as follows:
imagining that an adversary had been allowed to contribute three poisoned
training elements, we were able to verify that eight test set instances could
not have had their classifications changed (we checked nineteen at this stage).
These experiments took on average 0.15s, using on average 4243KB memory.

The experiment.sh script populated the `bench/iris/d2_V/` directory with many
.jsonl files (see `ls bench/iris/d2_V/`).
Internally, experiment.sh appends to these .jsonl files, so repeatedly invoking
the command when bench/iris/d2_V/ contains files other than the initial
"initcommands...txt" file will create unusual behavior.
The table above can always be recreated from the existing data via
`python3 scripts/data-wrangle/summarize.py $(ls bench/iris/d2_V/*.jsonl) | column -t -s ,`
(the pipe to `column` is just for readability).

This concludes ensuring the basics of experimentation function correctly;
at this point you may `exit` the container.


### 1.5 Additional Notes About Docker

A docker "container" is like a virtual machine whose initial internal state
is given from some specified docker "image."
You can have many containers that run from the same initial image,
each of which runs independently and has its own storage.

The `docker load` command only needs to be performed once.
Docker then maintains a copy of antidote:ae in its internal image storage;
the antidote-image.tar file can be deleted after the load command, and any
number of containers can still be made from the image.
Eventually, when you are finally done with all of the containers/evaluation,
you can remove the image from your system with `docker rmi antidote:ae`
(although you will need to remove all containers based on the image first)
and confirm it is then absent from `docker images`.

Previously, we started a container with the `--rm` flag so that it would delete
itself upon exit (for easy cleanup).
To create persistent containers that can later be resumed, omit this flag
(and for convenience use the `--name NAME` functionality).
Resume such containers with `docker start -i NAME`;
you can see a list of all your containers with `docker ps -a`;
manually delete containers you no longer need (which includes their contents!)
with `docker rm NAME`.











# 2 Step-by-Step Instructions

This section provides a bit more documentation about our tool and instructions
to reproduce the results presented in our paper. Completing all the steps should
walk the reader through:

* How to use our tool in its most basic forms
* How to run the experimental pipeline to produce the results in the paper
* How to adapt the set up to support additional datasets




### 2.1 Antidote Details

The name "Antidote" generally refers to our main executable `bin/main`, which
implements our algorithms "DTrace" (Section 3.3) and "DTrace#" (Section 4.3),
including the expressivity and precision improvements for DTrace# (Section 5).


#### 2.1.1 Summary of Revelant Flags

Here we describe the command line parameters for the tool necessary to run its
basic functionalities and to produce the results presented in the paper.

* `-f DIRECTORY ID`:
  This specifies which dataset to use.

  DIRECTORY is a path to the folder in which the dataset file is located
  (for this image, it's always /antidote/data).

  ID is string that the tool associates with metadata about each dataset
  (e.g. the training/test-set splits, the number of features...). In the paper,
  we test on the values `iris`, `mammography`, `cancer`, `mnist_simple_1_7`, and
  `mnist_1_7` (the Usage Information output of the tool includes a list of all
  supported IDs that can be used).
  To be explicit, we provide the relation of names in the paper (Section 6.1 and
  Table 1) to the names of the ID arguments:

        Paper Name                             ID argument
        ==========                             ===========
        Iris                                   iris
        Mammographic Masses                    mammography
        Wisconsin Diagnostic Breast Cancer     cancer
        MNIST-1-7-Binary                       mnist_simple_1_7
        MNIST-1-7-Real                         mnist_1_7

  Note that in some parts of the data-wrangling pipeline, there is a display
  inconsistency where experiments on the `cancer` ID yield data that refers to
  the string "wdbc".
  See the end of this document for notes on running on external datasets.

* -d DEPTH:
  This specifies the maximum depth of the tree to be learned.
  DEPTH should be a non-negative integer. 0 performs no splits of the training
  data (not very useful), 1 performs one split, and so on.

* -t TESTINDEX:
  This specifies which test index to run on.
  TESTINDEX should be a non-negative integer (0-indexed) smaller than the number
  of test set elements. There is not an explicit interface to check the range of
  test indices, but the tool will exit gracefully on those outside of bounds.

* -T:
  This instructs the tool to perform multiple runs, one for each element in the
  test set.

* -a NUM_DROPOUT:
  This instructs the tool to perform the poisoning-robustness analysis using the
  abstract transformers described in the paper, Section 4.4 (and referred to as
  the "box domain" in Section 6.3).

  NUM_DROPOUT should be a non-negative integer specifying the poisoning amount
  (*n* in the paper).

* -V NUM_DROPOUT:
  The same as the -a flag, but using the disjunctive version of the abstraction
  as defined in Section 5.2.

The tool exits with an error if necessary flags are not included or if multiple
conflicting flags are provided. Of the flags above,
Exactly one of `-t TESTINDEX` and `-T` needs to be used, and at most one of
`-a NUM_DROPOUT` and `-V NUM_DROPOUT` may be used (when neither is present, the
tool performs the concrete training/classification algorithm).

Additionally, the -d, -t, -a, and -V flags support sets of arguments, where all
combinations of the parameters are run. For example, the single command
`bin/main -f data iris -d "1 2" -t "0 1 2" -a "1 2"`
is equivalent to running all of
```
bin/main -f data iris -d 1 -t 0 -a 1
bin/main -f data iris -d 1 -t 0 -a 2
bin/main -f data iris -d 1 -t 1 -a 1
bin/main -f data iris -d 1 -t 1 -a 2
bin/main -f data iris -d 1 -t 2 -a 1
bin/main -f data iris -d 1 -t 2 -a 2
bin/main -f data iris -d 2 -t 0 -a 1
bin/main -f data iris -d 2 -t 0 -a 2
bin/main -f data iris -d 2 -t 1 -a 1
bin/main -f data iris -d 2 -t 1 -a 2
bin/main -f data iris -d 2 -t 2 -a 1
bin/main -f data iris -d 2 -t 2 -a 2
```


#### 2.1.2 Example Usage: Proving Poisoning Robustness

In the Getting Started section, we invoked the tool to classify an particular
test set instance of an iris using `bin/main -f data iris -d 2 -t 0`.
This performed the concrete learning algorithm described in our paper, Section
3.3 (this is the "DTrace" from Figure 4).
Our tool can take additional flags that run the abstract version, DTrace#,
which symbolically trains on all of the training sets in a particular "poisoning
model" (see Definition 3.1 and Section 4.1).

If we assume an adversary contributed up to 1 poisoned training element,
we can modify the passed parameters to perform this computation with the "box"
domain (as described in the paper, Section 4) -- specifically, the "-a N" flag:
`bin/main -f data iris -d 2 -t 0 -a 1 | jq`
The program should again terminate quickly, but with slightly different output:

    ...
      "posterior": {
        "Iris-setosa": [ 0, 1 ],
        "Iris-versicolor": [ 0, 1 ],
        "Iris-virginica": [ 0, 0 ]
      },
      "possible_classifications": [
        "Iris-setosa",
        "Iris-versicolor",
        "Iris-virginica"
      ]
    }

In this case, the analysis reports that many things are possible.
Each probability in the posterior distribution can take an *interval* of values
(written as `[lower bound, upper bound]`);
here, "Iris-virginia" is definitely assigned 0 probability, but each of the
other two classes could be assigned any probability between 0 and 1.
Accordingly, "possible_classifications" soundly reports that any of the classes
could have maximal probability, and we are not able to prove robustness.

Later in the paper (Section 5.2), we describe a variant of our abstraction that
uses disjuncts to perform a more precise analysis (but more resource-expensive).
This uses the "-V N" flag (instead of "-a"). If we run this variant:
`bin/main -f data iris -d 2 -t 0 -V 1 | jq`
we get

    ...
      "posterior": {
        "Iris-setosa": [ 0.961538, 1 ],
        "Iris-versicolor": [ 0, 0.038462 ],
        "Iris-virginica": [ 0, 0 ]
      },
      "possible_classifications": [
        "Iris-setosa"
      ]
    }

This time, the analysis kept enough precision to conclude that Iris-Setosa is
always given probability *at least* 0.96, while the other classes are given
probability *at most* 0.03, so Iris-Setosa always dominates; we have proved
the poisoning-robustness property for n=1.




### 2.2 Recreating the Paper's Experiments

In this section, we will walk through recreating all of the experimental claims
presented in the paper.
However (as is noted in the paper), all of our experiments were performed on
machines with 160GB of RAM. The total running time for all benchmarks amounts
to around 800 cpu hours: this is in large part due to *how long* it takes to
run out of memory, in the test cases that do so, when there is 160GB available.

Running the experiments with a smaller memory limit (and timeout period) will
drastically reduce the total running time, although this will naturally reduce
the number of verification problems that the tool is able to prove.
In each of the following sections, where appropriate, we describe how to run
versions of the experiments at smaller scales.
For completeness, we have also included the raw output of the verification
experiments as they performed on our machines (described later).


#### 2.2.1 Test Set Accuracies (Concrete Semantics)

Section 6.1, Table 1 reports test-set accuracies for each of the combinations
of datasets and tree depths that we consider.
This computation requires running the concrete learner on each element of the
test set (which our tool is not optimized for, and thus takes a while on some
of the datasets). In general, these experiments are much less expensive than the
verification problems in the next section.

To compute the test set accuracy of a depth-2 tree on iris, run:
```
bin/main -f data iris -d 2 -T > temp.jsonl
python3 scripts/data-wrangle/accuracy.py temp.jsonl
```
This should report `0.9` (meaning 90% accuracy).
(`rm temp.jsonl` to clean up.)

We have a script to compute the test set accuracies for all our combination
of parameters: it stores the raw output of the main tool in jsonl files in
bench/concrete, and it invokes the above accuracy.py script on each.
Run `scripts/batch-exp/test_accuracy.sh`. The results for the iris, mammography,
and cancer datasets should complete in seconds; the mnist variants take far
longer, around two hours combined, on our 4GHz machine. (Memory is not a
limiting factor for concrete training: we used a maximum of 150MB for this).

The results should match the numbers that appear in the table in the paper.
The accuracies can be recomputed quickly by pointing
scripts/data-wrangle/accuracy.py to the appropriate bench/concrete/*.jsonl file;
all can be done with:
`for F in $(ls bench/concrete/*.jsonl); do echo $F: $(python3 scripts/data-wrangle/accuracy.py $F); done`

(((TODO include our results for this)))

Because the memory overhead for this process is negligible (and our tool is
single-threaded), you can leave it running and continue to the next section with
a new docker container, if you wish to continue immediately.


#### 2.2.2 Recreating full Benchmarks (Abstract Semantics)

To produce the results discussed in the paper, we must recreate the experimental
setup described in Section 6.1, "Experimental Setup," in which we incrementally
increase the amount of poisoning, testing to see for how large of a poisoning
amount our tool is able to verify robustness.

For a specific dataset, tree depth, and abstract domain, the script (used in
the Getting Started Section) scripts/batch-exp/experiment.sh will perform this
iterative experiment. Another file, scripts/batch-exp/run_all.sh, invokes this
experiment script on each of the 40 combinations of depth, dataset, and domain.
If you have 800 hours to spare on a 160GB machine, you may run the following
command (although this is not recommended):
`scripts/batch-exp/run_all.sh 140000 3600`
which kills any individual execution of bin/main that exceeds 140000MB RAM or
runs for longer than 3600 seconds (an hour).
Since run_all.sh simply repeatedly invokes experiment.sh, this populates the
bench/abstract/ directory hierarchy with many .jsonl files.
(((TODO you may see the results of our execution by unzipping the thing)))

Note that the memory and time limits are implemented hackily through polling
(see `cat scripts/batch-exp/run_with_mem_limit.sh` if interested).
In particular, the memory limit is based on the amount of *RAM* used by *just
this process*, and will not function correctly if the operating system begins
swapping some of the data to other virtual memory. Accordingly, you should set
a limit a bit below the amount of RAM on your system (to accomodate for memory
used by the operating system and other processes); we used a 140GB limit on the
160GB machines; a 4GB limit is likely fine on an 8GB machine. The memory limit
is important to prevent thrashing, etc: without it, our systems tended to hang
when runs of Antidote using the "-V" flag took exponential amounts of memory.

If you give much more modest amounts of resources per run, timeouts and memory-
outs happen much sooner, and the experiments finish faster. The command
`scripts/batch-exp/run_all.sh 4000 120`
runs all of the experiments with at most 4GB RAM and 120s per instance.
On a 4GHz machine, the approximate run times for the benchmarks breaks down as:
(((TODO get this information)))

Finally, you could modify the run_all.sh file by deleting the lines of any sets
of experiments you wish not to run (for example, any lines for `mnist_1_7`, the
real-valued version of MNIST that is by far the most expensive benchmark),
and/or you could remove some of the initial commands in the
bench/abstract/.../initcommands.txt files, each of which corresponds to a
single test input, to reduce the total number of verification problems run.
(You would need to first `apt-get update && apt-get install [a text editor]`.)
You can always reconstruct the original run_all.sh file by running
`scripts/batch-exp/make_run_all.sh`.
Similarly, you can restore the original initcommands...txt files by running
`rm -rf bench/abstract && scripts/batch-exp/initcommands.sh`
---but note this also deletes any existing results produced by experiment.sh.
(As previously mentioned in the Getting Started Guide, this is the way to ensure
experiment.sh behaves correctly in the future, since it reads from any existing
*.jsonl files in those directories to inductively run further experiments.)

The most expensive benchmarks come from the cancer (wdbc, not mammography) and
mnist (both versions) datasets. If you do wish to recreate the full experimental
evaluation with larger memory and timeouts, we recommend spinning up multiple
docker containers (on different machines, since RAM is the limited resource)
and running individual `scripts/batch-exp/experiment.sh MEM TIME COMMANDSFILE`
invocations on each of them.
(If you do this, you will have to use docker copy commands to get all of the
results files into a single container to use the remainder of the data-wrangling
pipeline.)

Once you have completed running these scripts that create many scattered .jsonl
files, you should use our data-wrangling script that preprocesses and collects
all of the results into a single .jsonl file for use in the remaining sections:
`python3 scripts/data-wrangle/consolidate.py $(find bench/abstract -type f -name "*.jsonl") > bench/all.jsonl`
(((TODO do this for the provided results as well)))


#### 2.2.3 Reproducing Figures

Figure generation for summary Section 6.2, Figure 6

Figure generation for Section 6.3, Figure 7 (and supp figures?)


#### 2.2.4 Reproducing In-Text Quantitative Claims

* At the end of the paper's introduction, and repeated in Section 2, "An
  Involved Example," we give an example of an MNIST test instance and state:
  "Antidote proves that it is poisoning robust (always classified as a seven)
  for up to 192 poisoned elements in 90 seconds."
  To reproduce this, run
  `time bin/main -f data mnist_simple_1_7 -d 3 -t 511 -V 192`
  although note that this computation uses around 4.5GB of memory.
  Alternatively, the output for this run from our raw data can be found via
  `grep "\"dataset\": \"mnist_simple_1_7\", \"depth\": 3, \"test_index\": 511, \"domain\": \"disjuncts\", \"num_dropout\": 192," all.jsonl | jq`
  (((TODO make sure that this matches the all.jsonl naming---actually, everywhere in the section)))

For the remaining claims, we have provided a single script that extracts the
relevant data for all of what follows (though much of it can be taken directly
from the plots produced prior):
`python3 scripts/data-wrangle/paperstats.py all.jsonl`
(You may substitute the all.jsonl that you compute, which is likely less
complete, and may see reduced numbers of verified instances and more timeouts.)
The output corresponds to each of the following claims, in order.

* In Section 6.2, we summarize that we prove 38 verification problems for
  `mnist_1_7`, depth=2, n=64, with 800s average runtime.

* In Section 6.3, "Box vs Disjuncts," we state that on `mnist_simple_1_7` with
  depth=3 and n=64, disjuncts verifies 52 problems (avg 32s, 1650MB) while box
  verifies 15 problems (0.7s, 150MB).
  We state, contrasting, that for depth=4, n=128, disjuncts always timeouts,
  while box is able to verify one problem instance.

* In Section 6.3, "Number of Poisoned Elements," we state that, out of all of
  the experiments conducted, 95% of instances using the box domain finished
  within 20s, none timed out, and the longest took 232s.

* In Section 6.3, "Size of Dataset and Number of Features," we state average
  runtimes for each of the datasets using disjuncts, depth=3, n=0.5% as follows:
  `iris` 0.1s, `mammography` 0.2s, `cancer` 26s, `mnist_simple_1_7` 32s, and
  `mnist_1_7` always times out.

* In Section 6.3, "Depth of the Tree," we state average runtimes for the binary
  MNIST variant using disjuncts and n=64, for varying depths:
  depth 1 as 0.3s, 2 as 0.5s, 3 as 32s, and 4 as 933s.


### 2.3 Running on Other Datasets

TODO have to modify code, so you'll need to also install things.
Pipeline for batch experimentation and figure generation is hard-coded
for filenames in the paper and would take some dedicated hacking to extend.


#### 2.3.1 Supported Feature Types


#### 2.3.2 Compiling the Code


#### 2.3.3 Modifying the Source to Support New Datasets
