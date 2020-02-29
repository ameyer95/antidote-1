import json
import math
import os
import sys
from collections import namedtuple
from functools import reduce
import matplotlib.pyplot as plt
import matplotlib.ticker as mticker

datasets = ["iris", "mammography", "wdbc", "mnist_simple_1_7", "mnist_1_7"]
depths = [1, 2, 3, 4]
domains = ["box", "disjuncts"]

test_size = {"iris" : 30, "mammography" : 166, "wdbc" : 113, "mnist_simple_1_7" : 100, "mnist_1_7" : 100}



##
# Basic file-manipulating functions
##

def read_jsonl(filename):
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()
    return [json.loads(line) for line in lines]

def touch_directory(directory):
    if not os.path.exists(directory):
        os.makedirs(directory)

# header should be a string, rows should be foldable
def write_csv(filename, header, rows):
    f = open(filename, 'w')
    f.write(header + "\n")
    for row in rows:
        f.write(reduce(lambda x,y : str(x) + "," + str(y), row) + "\n")
    f.close()



##
# Code to generate the first summary graphs that show,
# for each dataset, how many benchmarks we're able to prove
# as a function of the poisoning parameter.
##

# We consider 0 to be a power of two to simplify other code
def power_of_two(n):
    if n == 0:
        return True
    ell2 = int(math.log2(n))
    return 2**ell2 == n

# Give the json_lines restricted to a particular dataset and depth,
# returns a list of (num_dropout, # verified) pairs (in ascending num_dropout order)
# that considers runs using either domain
def proven_vs_poisoned(json_lines):
    # for each num_dropout, track which results were verified
    n_to_verified = {}
    for x in json_lines:
        if x['num_dropout'] not in n_to_verified:
            n_to_verified[x['num_dropout']] = set()
        if x['verified']:
            n_to_verified[x['num_dropout']].add(x['test_index'])
    # turn this into a list of (num_dropout, #verified)
    ret = sorted([(k,len(v)) for k,v in n_to_verified.items()])
    # because we're aggregating both the single- and many-disjuncts cases,
    # whichever performed worse began backwards-binary searching (so num_dropout is not a power of two),
    # so we want to discard all non-powers-of-two smaller than the largest power-of-two with any verified
    temp = [n for n,v in ret if power_of_two(n) and v > 0]
    thresh = max(temp) if len(temp) > 0 else 0
    ret = [(n,v) for n,v in ret if power_of_two(n) or n >= thresh]
    # Additionally, we don't need extraneous tail points (keep only one zero-verified)
    zero_verified = [i for i in range(len(ret)) if ret[i][1] == 0]
    for i in reversed(zero_verified[1:]):
        del ret[i]
    return ret

def create_proven_vs_poisoned_csvs(json_lines):
    path = "proven_vs_poisoned"
    touch_directory(path)
    for dataset in datasets:
        dataset_lines = [x for x in json_lines if x['dataset'] == dataset]
        for depth in depths:
            pairs = proven_vs_poisoned([x for x in dataset_lines if x['depth'] == depth])
            # make the number verified a percentage
            pairs = [(n, v/test_size[dataset]) for n,v in pairs]
            filename = dataset + "_d" + str(depth) + ".csv"
            write_csv(path + "/" + filename, "num_dropout,percent_verified", pairs)

def create_proven_vs_poisoned_plots(json_lines, path_prefix):
    touch_directory(path_prefix)
    marker_map = {1 : 'o', 2 : 's', 3 : '^', 4 : '*'}
    color_map = {1 : 'b', 2 : 'r', 3 : 'tab:brown', 4 : 'k'}
    for dataset in datasets:
        dataset_lines = [x for x in json_lines if x['dataset'] == dataset]
        for depth in depths:
            pairs = proven_vs_poisoned([x for x in dataset_lines if x['depth'] == depth])
            # make the number verified a percentage
            pairs = [(n, v/test_size[dataset]) for n,v in pairs if n != 0]
            if len(pairs) > 0:
                xs,ys = tuple(zip(*pairs))
                plt.plot(xs, ys, label="depth " + str(depth), marker=marker_map[depth], color=color_map[depth], clip_on=False)
        plt.title("Proven vs Poisoned: " + dataset)
        plt.xlabel("poisoning n (log scale)")
        plt.xscale('log', basex=2)
        plt.gca().xaxis.set_major_formatter(mticker.StrMethodFormatter('{x:.0f}'))
        plt.ylabel("fraction verified")
        plt.ylim(0, 1)
        plt.legend()
        dest = path_prefix + "/proven_vs_poisoned_" + str(dataset) + ".pdf"
        plt.savefig(dest)
        print("written", dest)
        plt.clf()


##
# Code to generate the exhaustive reports for each combination
# of depth, dataset, and domain.
# We're talking 60 graphs (but not quite so many csv files).
##

StatsTuple = namedtuple('StatsTuple', ['num_dropout', 'num_verified', 'avg_time', 'avg_max_memory'])
StatsPoint = namedtuple('StatsPoint', ['test_index', 'verified', 'time', 'memory'])

# Give the jsonl_lines restricted to a particular dataset, depth, and domain.
# Returns a list of (num_dropout, # verified, avg (non-oom/to) time, avg (non-oom/to) max mem)
# tuples in ascending num_dropout order
def stats_vs_poisoned(json_lines):
    # for each num_dropout, track which results terminated
    n_to_terminated = {}
    for x in json_lines:
        if x['num_dropout'] not in n_to_terminated:
            n_to_terminated[x['num_dropout']] = set()
        if not x['oom']:
            n_to_terminated[x['num_dropout']].add(StatsPoint(x['test_index'], x['verified'], x['time'], x['memory']))
    ret = []
    for n,xs in n_to_terminated.items():
        num_verified = len([x for x in xs if x.verified])
        times = [float(x.time) for x in xs]
        avg_time = sum(times)/len(times) if len(times) > 0 else "nan"
        memory = [int(x.memory)/1000 for x in xs] # converted to MB
        avg_max_memory = sum(memory)/len(memory) if len(memory) > 0 else "nan"
        ret.append(StatsTuple(n, num_verified, avg_time, avg_max_memory))
    ret = sorted(ret, key=lambda t : t.num_dropout)
    # keep only at most one zero-verified point
    zero_verified = [i for i in range(len(ret)) if ret[i].num_verified == 0]
    for i in reversed(zero_verified[1:]):
        del ret[i]
    return ret

def create_stats_vs_poisoned_csvs(json_lines):
    header = reduce(lambda x,y : x + "," + y, StatsTuple._fields)
    path = "exhaustive"
    touch_directory(path)
    for dataset in datasets:
        dataset_lines = [x for x in json_lines if x['dataset'] == dataset]
        for depth in depths:
            dataset_depth_lines = [x for x in dataset_lines if x['depth'] == depth]
            for domain in domains:
                these_lines = [x for x in dataset_depth_lines if x['domain'] == domain]
                tuples = stats_vs_poisoned(these_lines)
                filename = dataset + "_d" + str(depth) + "_" + domain + ".csv"
                write_csv(path + "/" + filename, header, tuples)

def create_stats_vs_poisoned_plots(json_lines, path_prefix):
    touch_directory(path_prefix)
    for dataset in datasets:
        dataset_lines = [x for x in json_lines if x['dataset'] == dataset]
        fig, axs = plt.subplots(3, len(depths), sharex=True, sharey='row', figsize=(14,8))
        fig.suptitle(dataset)
        axs[0, 0].set_ylabel("# Verified")
        if dataset not in {'iris', 'mammography'}:
            axs[1, 0].set_ylabel("Avg Time (s) (log scale)")
            axs[1, 0].set_yscale('log')
        else:
            axs[1, 0].set_ylabel("Avg Time (s)")
        if dataset != 'iris':
            axs[2, 0].set_ylabel("Avg Max Memory (MB) (log scale)")
            axs[2, 0].set_yscale('log')
        else:
            axs[2, 0].set_ylabel("Avg Max Memory (MB)")
        plt.xscale('log', basex=2)
        plt.gca().xaxis.set_major_formatter(mticker.StrMethodFormatter('{x:.0f}'))
        for i in range(len(depths)):
            depth = depths[i]
            axs[0, i].set_title("Depth " + str(depth))
            axs[2, i].set_xlabel("Poisoning n (log scale)")
            for domain in domains:
                these_lines = [x for x in dataset_lines if x['depth'] == depth and x['domain'] == domain]
                stats = stats_vs_poisoned(these_lines)
                stats = [tuple(stat) for stat in stats if stat[0] != 0]
                if len(stats) > 0:
                    ns,vs,ts,ms = tuple(zip(*stats))
                    ts = [t if t != "nan" else None for t in ts]
                    ms = [m if m != "nan" else None for m in ms]
                    for axisrow,ptsy in [(0, vs), (1, ts), (2, ms)]:
                        thismarker = 'o' if domain == 'box' else 's'
                        thiscolor = 'b' if domain == 'box' else 'r'
                        axs[axisrow, i].plot(ns, ptsy, label=domain, marker=thismarker, color=thiscolor)
        fig.legend(labels=domains)
        dest = path_prefix + "/stats_vs_poisoned_" + str(dataset) + ".pdf"
        plt.savefig(dest)
        print("written", dest)
        plt.clf()


##
# main
##

def make_csvs():
    json_lines = read_jsonl("all.jsonl")
    create_proven_vs_poisoned_csvs(json_lines)
    create_stats_vs_poisoned_csvs(json_lines)

def make_pdfs(all_jsonl, path_prefix):
    json_lines = read_jsonl(all_jsonl)
    create_proven_vs_poisoned_plots(json_lines, path_prefix)
    create_stats_vs_poisoned_plots(json_lines, path_prefix)

if __name__ == '__main__':
    #make_csvs()
    jsonl_file = sys.argv[1]
    file_prefix = sys.argv[2]
    make_pdfs(jsonl_file, file_prefix)
