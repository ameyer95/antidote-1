import os
import pandas as pd
import numpy as np
import sys

def load_data(file):
    if file[-4:] == "json":
        try:
            result = pd.read_json(file, lines=True)
            data = data.append(result, ignore_index=True)
        except ValueError as e:
            pass
    else:
        print("File type must be json and must contain the directory path")
    return data

def get_certified_pct(data):
    ''' Returns three values: (1) the percent of data that was certified AND correct,
    (2) the percent of data that was certified, and
    (3) a list of indices (i.e., test sample indices) that are NOT certifiably robust. 
    Compatible with any dataset with integer labels. '''
    correct = 0
    certified = 0
    indices = []
    count = len(data)
    for index,row in data.iterrows():
        if len(row["possible_classifications"]) == 1:
            certified+=1
            if int(row["possible_classifications"][0]) == int(row["ground_truth"]):
                correct+=1
        else:
            indices.append(row['test_index'])

    return correct/count, certified/count, indices

def get_certified_pct_mnist(data):
    ''' Returns three values: (1) the percent of data that was certified AND correct,
    (2) the percent of data that was certified, and
    (3) a list of indices (i.e., test sample indices) that are NOT certifiably robust. 
    Compatible specifically with MNIST 1/7 (a dataset where the classes are strings)'''
    correct = 0
    certified = 0
    indices = []
    count = len(data)
    for index,row in data.iterrows():
        if len(row["possible_classifications"]) == 1:
            certified+=1
            if row["possible_classifications"][0] == row["ground_truth"]:
                correct+=1
        else:
            indices.append(row['test_index'])

    return correct/count, certified/count, indices

def main():
    filename = sys.argv[0]
    mnist = sys.argv[1]
    if mnist:
        cert_correct, cert, indices = get_certified_pct_mnist(filename)
    else:
        cert_correct, cert, indices = get_certified_pct(filename)

    print(cert)

if __name__ == "__main__":
    main()