from functools import reduce
import json
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

def incremented_experiment(json_lines):
    ret = []
    for line in json_lines:
        tokens = line['command'].split()
        # look for the index of "-a" or "-V" and double the integer at the next index
        try:
            i = tokens.index("-a")
        except ValueError:
            i = tokens.index("-V")
        int_index = i + 1
        prev_value = int(tokens[int_index])
        next_value = 2*prev_value if prev_value != 0 else 1
        tokens[int_index] = str(next_value)
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
        return incremented_experiment(filter_inconclusive(filter_oom(js)))

if __name__ == '__main__':
    filename = sys.argv[1] # the previous run's jsonl file OR a list of commands in "initcommands_*"
    for e in next_experiment(filename):
        print(e)
