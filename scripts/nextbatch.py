from functools import reduce
import json
import math
import sys

def load_jsonl(filename):
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()
    return [json.loads(line) for line in lines]

def filter_oom(json_lines):
    return [line for line in json_lines if line['normal_exit']]

def filter_inconclusive(json_lines):
    return [line for line in json_lines if len(line['result']['possible_classifications']) == 1]

def power_of_two(n):
    ell2 = int(math.log2(n))
    return 2**ell2 == n

def binary_backwards(n):
    ell2 = int(math.log2(n-1))
    smaller = 2**ell2
    return int((smaller + n) / 2)

# Read in the previous experiment results.
# Look at the num_dropout parameter and possibly increment it:
# - If it was a power of two and any of the previous experiments succeeded,
#   then we double the parameter and run those successful instances next.
# - If it was a power of two and all of the previous experiments failed,
#   we binary search backwards and rerun all of the previous experiments
#   (unless that parameter was 2, because we'd be redoing 1).
# - If it was not a power of two and all of the previous experiments failed,
#   we do as in the last case, except we similarly stop
#   if the new parameter would be a power of 2.
# - If it was not a power of two and any of the previous experiments succeeded,
#   we stop.
def incremented_experiment(json_lines):
    json_successes = filter_inconclusive(filter_oom(json_lines))
    line_tokens = [line['command'].split() for line in json_lines]
    prev_value = set()
    for tokens in line_tokens:
        try:
            i = tokens.index("-a")
        except:
            i = tokens.index("-V")
        prev_value.add(int(tokens[i+1]))
    assert len(prev_value) == 1
    prev_value = list(prev_value)[0]

    any_successes = len(json_successes) > 0
    next_value = None
    next_json = None
    if prev_value == 0 or power_of_two(prev_value):
        if any_successes:
            next_value = 2 * prev_value if prev_value != 0 else 1
            next_json = json_successes
        elif prev_value > 1: # really checking != 0
            next_value = binary_backwards(prev_value)
            if not power_of_two(next_value):
                next_json = json_lines
    elif not any_successes and prev_value > 1:
        next_value = binary_backwards(prev_value)
        if not power_of_two(next_value):
            next_json = json_lines

    ret = []
    if next_json is not None:
        for line in next_json:
            tokens = line['command'].split()
            # look for the index of "-a" or "-V" and double the integer at the next index
            try:
                i = tokens.index("-a")
            except ValueError:
                i = tokens.index("-V")
            tokens[i+1] = str(next_value)
            ret.append(reduce(lambda x,y : x + " " + y, tokens))
    return ret

def next_experiment(filename):
    prefix = "initcommands_";
    filename_nopath = filename.split("/")[-1]
    if filename_nopath[:len(prefix)] == prefix:
        f = open(filename, 'r')
        lines = [line.strip() for line in f.readlines()] # remove trailing newline
        f.close()
        return lines
    else:
        js = load_jsonl(filename)
        return incremented_experiment(js)

if __name__ == '__main__':
    filename = sys.argv[1] # the previous run's jsonl file OR a list of commands in "initcommands_*"
    for e in next_experiment(filename):
        print(e)
