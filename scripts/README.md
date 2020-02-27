# Scripts to automate experiment batches and collect data

## Running the Experiment Pipeline

The main script to be invoked is batch-exp/experiment.sh,
which performs the incremental poisoning-increasing experiment for a particular configuration
of dataset, depth, and abstract domain.
As documented in its comments, it takes a memory limit and a time limit as arguments
(these limits are applied to each individual execution of the main tool).

To run all of the experiments used in our PLDI paper,
first execute `batch-exp/make_run_all.sh` to generate the batch-exp/run\_all.sh file,
which simply contains every combination of invocations to batch-exp/experiment.sh.
Before running this new file, be sure to have created the top-level bench directory structure
via `batch-exp/initcommands.sh`.

## Data Wrangling

Example usage of [data-wrangle/summarize.py](data-wrangle/summarize.py) script,
assuming [batch-exp/experiment.sh](batch-exp/experiment.sh) has produce a number of
.jsonl files in the top-level bench directory:
from this directory (with `column` installed, for nice formatting),
```
python3 data-wrangle/summarize.py $(ls ../bench/mnist_simple_1_7/d3_V/*.jsonl) | column -t -s ,
```
will produce a summary of how many test instances are provably robust
(as well as information about their average time and memory usage)
as we increase the poisoning amount.
Note that this command is invoked at the end of batch-exp/experiment.sh for convenience.
