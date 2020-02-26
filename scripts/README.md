# Scripts to automate experiment batches and collect data

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
