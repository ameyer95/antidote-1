# Recreates the in-text claims from the paper.
# Point to a single all.jsonl file produced by scripts/data-wrangle/consolidate.py

import json
import sys

def read_jsonl(filename):
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()
    return [json.loads(line) for line in lines]

def average_runtime(json_lines):
    runtimes = [x['time'] for x in json_lines if not x['oom']]
    return sum(runtimes) / len(runtimes) if len(runtimes) > 0 else "N/A"

def average_memory(json_lines):
    runmems = [x['memory'] for x in json_lines if not x['oom']]
    return sum(runmems) / len(runmems) if len(runmems) > 0 else "N/A"

def claim_mnist_real_d2_n64(json_lines):
    print("Power of DTrace# on MNIST-1-7-Real, depth=2, n=64:")
    these_lines = [x for x in json_lines if x['dataset'] == 'mnist_1_7' and x['depth'] == 2 and x['num_dropout'] == 64]
    box_lines = [x for x in these_lines if x['domain'] == 'box']
    box_verif = [x for x in box_lines if x['verified']]
    v_box = {x['test_index'] for x in box_verif}
    disjuncts_lines = [x for x in these_lines if x['domain'] == 'disjuncts']
    disjuncts_verif = [x for x in disjuncts_lines if x['verified']]
    v_disjuncts = {x['test_index'] for x in disjuncts_verif}
    print("  # verified by box:", len(box_verif), "(" + str(len(box_lines)), "attempted)")
    print("  # verified by disjuncts:", len(disjuncts_verif), "(" + str(len(disjuncts_lines)), "attempted)")
    print("  # verified combined:", len(v_box | v_disjuncts)) # set union since there could be overlap
    print("  average runtime for verified instances (s):", average_runtime(disjuncts_verif + box_verif))

def claim_box_vs_disjuncts(json_lines):
    print("Power and performance of disjuncts vs box on MNIST-1-7-Binary, depth=3, n=64:")
    these_lines = [x for x in json_lines if x['dataset'] == 'mnist_simple_1_7' and x['depth'] == 3 and x['num_dropout'] == 64]
    box_lines = [x for x in these_lines if x['domain'] == 'box']
    box_verif = [x for x in box_lines if x['verified']]
    disjuncts_lines = [x for x in these_lines if x['domain'] == 'disjuncts']
    disjuncts_verif = [x for x in disjuncts_lines if x['verified']]
    print("  # verified by box:", len(box_verif), "(" + str(len(box_lines)), "attempted)")
    print("  average time (s): ", average_runtime(box_lines))
    print("  average max mem (KB): ", average_memory(box_lines))
    print("  # verified by disjuncts:", len(disjuncts_verif), "(" + str(len(disjuncts_lines)), "attempted)")
    print("  average time (s): ", average_runtime(disjuncts_lines))
    print("  average max mem (KB): ", average_memory(disjuncts_lines))
    print("But on MNIST-1-7-Binary, depth=4, n=128:")
    these_lines = [x for x in json_lines if x['dataset'] == 'mnist_simple_1_7' and x['depth'] == 4 and x['num_dropout'] == 128]
    box_lines = [x for x in these_lines if x['domain'] == 'box']
    box_verif = [x for x in box_lines if x['verified']]
    disjuncts_lines = [x for x in these_lines if x['domain'] == 'disjuncts']
    disjuncts_verif = [x for x in disjuncts_lines if x['verified']]
    print("  # verified by box:", len(box_verif), "(" + str(len(box_lines)), "attempted)")
    print("  # verified by disjuncts:", len(disjuncts_verif), "(" + str(len(disjuncts_lines)), "attempted, " + str(len([x for x in disjuncts_lines if x['oom']])), "timeout/oom)")
    

def claim_box_quick(json_lines):
    print("Box domain efficiency:")
    box_lines = [x for x in json_lines if x['domain'] == 'box']
    to_or_oom = [x for x in box_lines if x['oom']] # The batch experiment pipelin lumps OOM and TO together
    finished = [x for x in box_lines if not x['oom']]
    finished_lt20 = [x for x in finished if x['time'] <= 20]
    print("  # run using box:", len(box_lines))
    print("  # timeout or oom:", len(to_or_oom))
    print("  # finished:", len(finished))
    if len(finished) > 0:
        print("  # finished within 20s:", len(finished_lt20), "(" + str(100*len(finished_lt20)/len(finished))[:4] + "%)")
        print("  max non-TO runtime (s):", max([x['time'] for x in finished]))
    else:
        print("  # finished within 20s:", len(finished_lt20))
        print("  max non-TO runtime (s): N/A")

def claim_dataset_complexity(json_lines):
    print("Complexity scaling in dataset size and # features; disjuncts, depth=3, n=0.5%:")
    disj_d_lines = [x for x in json_lines if x['domain'] == 'disjuncts' and x['depth'] == 3]
    # iris: 120 -> .6 -> 1
    # mammography 664 -> 3.32 -> 2 (3 not run)
    # cancer 456 -> 2.28 -> 1 (no verif at 2)
    # mnist 13007 -> 65.035 -> 64
    ns = {"iris" : 1, "mammography" : 2, "cancer" : 1, "mnist_simple_1_7" : 64, "mnist_1_7" : 64}
    iris_lines = [x for x in disj_d_lines if x['dataset'] == 'iris' and x['num_dropout'] == ns["iris"]]
    mammography_lines = [x for x in disj_d_lines if x['dataset'] == 'mammography' and x['num_dropout'] == ns["mammography"]]
    cancer_lines = [x for x in disj_d_lines if x['dataset'] == 'wdbc' and x['num_dropout'] == ns["cancer"]]
    mnist_bin_lines = [x for x in disj_d_lines if x['dataset'] == 'mnist_simple_1_7' and x['num_dropout'] == ns["mnist_simple_1_7"]]
    mnist_real_lines = [x for x in disj_d_lines if x['dataset'] == 'mnist_1_7' and x['num_dropout'] == ns["mnist_1_7"]]
    for s,js in [("iris", iris_lines), ("mammography", mammography_lines), ("cancer", cancer_lines), ("mnist_simple_1_7", mnist_bin_lines), ("mnist_1_7", mnist_real_lines)]:
        print("  " + s + ": n=" + str(ns[s]) + ", #run=" + str(len(js)) + ", average time (s):", average_runtime(js))

def claim_depth_complexity(json_lines):
    print("Time scaling in tree depth for MNIST-1-7-Binary, disjuncts, n=64:")
    these_lines = [x for x in json_lines if x['dataset'] == 'mnist_simple_1_7' and x['domain'] == 'disjuncts' and x['num_dropout'] == 64]
    for depth in [1,2,3,4]:
        d_lines = [x for x in these_lines if x['depth'] == depth]
        print("  Depth " + str(depth) + ":", "#run=" + str(len(d_lines)) + ", average time (s):", average_runtime(d_lines))

def all_claims(filename):
    json_lines = read_jsonl(filename)
    claims = [claim_mnist_real_d2_n64, claim_box_vs_disjuncts, claim_box_quick, claim_dataset_complexity, claim_depth_complexity]
    for claim in claims:
        claim(json_lines)
        print() # newline

if __name__ == "__main__":
    filename = sys.argv[1]
    all_claims(filename)
