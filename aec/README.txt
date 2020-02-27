# Getting Started Guide

The artifact is a docker image based on ubuntu:18.04,
and our testing of the artifact was done on a ubuntu:18.04 host machine
using Docker version 18.09.9, build 1752eb3.
Docker images are portable and run on a variety of operating systems,
but the instructions given in this guide will assume the user is operating
in a bash shell on a unix-like system.
All you should need to begin is the given antidote-image.tar file and a working
docker installation.


### Setting up Docker

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


### Running the Image

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


### Running the Tool

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


### Running Batches of Experiments

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


### Additional Notes About Docker

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






# Step-by-Step Instructions

TODO: everything

### Details of the Executables

#### Computing the Possible Classifications Under Poisoning

If we assume an adversary contributed up to 1 poisoned training element,
we can modify the passed parameters to perform this computation with the "box"
domain (as described in the paper, Section 4):
`bin/main -f data iris -d 2 -t 0 -a 1`
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
(written as [lower bound, upper bound]);
here, "Iris-virginia" is definitely assigned 0 probability, but each of the
other two classes could be assigned any probability between 0 and 1.
Accordingly, "possible_classifications" TODO

TODO show how -V is more precise

### Recreating the Paper's Experiments

### Running on Other Datasets
