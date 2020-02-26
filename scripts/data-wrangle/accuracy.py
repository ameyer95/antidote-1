# Takes one .jsonl filename as input.
# Each json object should be the direct output of the main tool bin/main
# (not some wrapped information from the scripts/batch-exp/ pipeline).

import json
import sys

def load_jsonl(filename):
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()
    return [json.loads(line) for line in lines]

def correct(json_obj):
    if len(json_obj["possible_classifications"]) != 1:
        return False
    return json_obj["possible_classifications"][0] == json_obj["ground_truth"]

def compute_accuracy(json_lines):
    count = 0
    for json_obj in json_lines:
        if correct(json_obj):
            count += 1
    return count / len(json_lines)

if __name__ == '__main__':
    filename = sys.argv[1]
    print(compute_accuracy(load_jsonl(filename)))
