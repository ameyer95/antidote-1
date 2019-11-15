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

def next_experiment(json_lines):
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

if __name__ == '__main__':
    filename = sys.argv[1] # the previous run's jsonl file
    js = load_jsonl(filename)
    for e in next_experiment(filter_inconclusive(filter_oom(js))):
        print(e)
