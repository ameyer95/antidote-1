#!/bin/bash

# Once all of the individual experiments' .jsonl output files have been made
# in an elaborate nested directory structure in [top level]/bench,
# this script simply calls the coalescing consolidate.py script
# passing all of the .jsonl files in any subdirectory of bench as arguments.
# Since consolidate.py dumps a lot of json lines to stdout,
# you probably want to redirect this script to a file.

cd $(dirname "$0") # Descend into the same directory as the script
python3 consolidate.py $(find ../../bench -type f -name "*.jsonl")
