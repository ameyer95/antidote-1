import json
import sys
from collections import namedtuple

def extract_domain_and_num_dropout(tokens):
    try:
        i = tokens.index("-a")
        domain = "box"
    except ValueError:
        i = tokens.index("-V")
        domain = "disjuncts"
    return domain, int(tokens[i+1])

def convert_time(hms_string):
    hms = hms_string.split(":")
    total = 0
    for unit in hms:
        total *= 60
        total += float(unit)
    return total

def name_map(name):
    if name in {"iris", "mammography", "mnist_simple_1_7", "mnist_1_7"}:
        return name
    elif name == "cancer":
        return "wdbc"
    else:
        assert False

def convert_json_line(json_line):
    tokens = json_line['command'].split()
    x = {}
    x['dataset'] = name_map(tokens[tokens.index("-f") + 2])
    x['depth'] = int(tokens[tokens.index("-d") + 1])
    x['test_index'] = int(tokens[tokens.index("-t") + 1])
    x['domain'], x['num_dropout'] = extract_domain_and_num_dropout(tokens)
    x['oom'] = not json_line['normal_exit']
    x['time'] = convert_time(json_line['time'])
    x['memory'] = int(json_line['memory'])
    x['verified'] = False if x['oom'] else (len(json_line['result']['possible_classifications']) == 1)
    return x

def load_jsonl(filename):
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()
    return [json.loads(line) for line in lines]

def consolidate(filenames):
    converted_lines = []
    for filename in filenames:
        json_lines = load_jsonl(filename)
        for line in json_lines:
            converted_lines.append(convert_json_line(line))
    dataset_order = { "iris" : 1, "mammography" : 2, "wdbc" : 3, "mnist_simple_1_7" : 4, "mnist_1_7" : 5}
    return sorted(converted_lines, key=lambda x : (dataset_order[x['dataset']], x['depth'], x['domain'], x['num_dropout'], x['test_index']))

if __name__ == '__main__':
    filenames = sys.argv[1:]
    lines = consolidate(filenames)
    #print(json.dumps(lines))
    for line in lines:
        print(json.dumps(line))
