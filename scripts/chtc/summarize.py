from collections import namedtuple
from functools import reduce
import json
import sys

class Data:
    __slots__ = ['num_run', 'num_verified', 'num_oom', 'total_time_finished', 'total_time_oom', 'total_memory_finished']
    def __init__(self):
        self.num_run = 0
        self.num_verified = 0
        self.num_oom = 0
        self.total_time_finished = 0 # in seconds
        self.total_time_oom = 0
        self.total_memory_finished = 0 # in KB

CSVRow = namedtuple('CSVRow', ['num_dropout', 'num_run', 'num_verified', 'num_oom', 'average_time_finished', 'average_time_all', 'average_memory_finished'])

def load_jsonl(filename):
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()
    return [json.loads(line) for line in lines]

def extract_num_dropout(json_line):
    tokens = json_line['command'].split()
    # look for the index of "-a" or "-V"
    try:
        i = tokens.index("-a")
    except ValueError:
        i = tokens.index("-V")
    int_index = i + 1
    return int(tokens[int_index])

def convert_time(time_string):
    hms = time_string.split(":")
    total = 0
    for unit in hms:
        total *= 60
        total += float(unit)
    return total

def compute_rows(filenames):
    counts = {}
    jsonl_files = [load_jsonl(f) for f in filenames]
    for jsonl_file in jsonl_files:
        for json_line in jsonl_file:
            num_dropout = extract_num_dropout(json_line)

            if num_dropout not in counts:
                counts[num_dropout] = Data()
            counts[num_dropout].num_run += 1

            if json_line['normal_exit']:
                if len(json_line['result']['possible_classifications']) == 1:
                    counts[num_dropout].num_verified += 1
                counts[num_dropout].total_time_finished += convert_time(json_line['time'])
                counts[num_dropout].total_memory_finished += int(json_line['memory'])
            else:
                counts[num_dropout].num_oom += 1
                counts[num_dropout].total_time_oom += convert_time(json_line['time'])
    csv_rows = []
    for k,v in counts.items():
        average_time_finished = v.total_time_finished / v.num_run
        average_time_all = (v.total_time_finished + v.total_time_oom) / v.num_run
        average_memory_finished = int(v.total_memory_finished / v.num_run)
        csv_rows.append(CSVRow(k, v.num_run, v.num_verified, v.num_oom, f'{average_time_finished:.2f}', f'{average_time_all:.2f}', average_memory_finished))
    return sorted(csv_rows, key=lambda x : x.num_dropout)

if __name__ == '__main__':
    filenames = sys.argv[1:]
    csv_rows = compute_rows(filenames)
    print(reduce(lambda x,y : x + "," + y, CSVRow._fields))
    for row in csv_rows:
        print(reduce(lambda x,y : x + "," + y, [str(i) for i in row]))
