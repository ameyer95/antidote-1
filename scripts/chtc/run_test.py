import sys

if __name__ == '__main__':
    filename = sys.argv[1]
    for e in next_experiment(filename):
        print(e)