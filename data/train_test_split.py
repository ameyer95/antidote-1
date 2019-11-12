# Performs an 80-20 randomized split of the UCI datasets.
# We preprocess this for reproducibility / determinization.
# There are two options:
#   (1) Reading the rows for the split from a file, or
#   (2) Generating a random row partitioning and accordingly
#       it in a file;
# however, there is no interface for selecting between them.
# The default behavior is (1), while (2) can be achieved
# by modifying the code herein.

from collections import namedtuple
import random

FileNames = namedtuple('FileNames', ['input_file', 'rand_file', 'output80_file', 'output20_file'])

iris = FileNames("bezdekIris.data", "iris.rand", "iris80.data", "iris20.data")
cancer = FileNames("wdbc.data", "wdbc.rand", "wdbc80.data", "wdbc20.data")
wine = FileNames("winequality.data", "winequality.rand", "winequality80.data", "winequality20.data")
yeast = FileNames("yeast.data", "yeast.rand", "yeast80.data", "yeast20.data")
retinopathy = FileNames("retinopathy.data", "retinopathy.rand", "retinopathy80.data", "retinopathy20.data")

Instance = namedtuple('Instance', ['file_names', 'random_seed'])

instances = { 'iris' : Instance(iris, 0), 'cancer' : Instance(cancer, 17), 'wine' : Instance(wine, 43), 'yeast' : Instance(yeast, 100), 'retinopathy' : Instance(retinopathy, 111) }

def readFile(filename):
    f = open(filename, 'r')
    lines = f.readlines()
    f.close()
    # Iris has an extra newline
    lines = [line.strip() for line in lines]
    return [line for line in lines if line != ""]

# lines don't have \n
def writeFile(filename, lines):
    f = open(filename, 'w')
    f.writelines([line + "\n" for line in lines])
    f.close()

def splitLines(lines, left_indices):
    left = [lines[i] for i in range(len(lines)) if i in left_indices]
    right = [lines[i] for i in range(len(lines)) if i not in left_indices]
    return left, right

# Rejection sampling
def pickNRandomIndices(total, N):
    ret = set()
    while len(ret) < N:
        ret.add(random.randrange(total))
    return ret

# Mode (2) from top comment
def performFreshSplit(instance):
    lines = readFile(instance.file_names.input_file)
    N = int(.2 * len(lines))
    random.seed(instance.random_seed)
    indices20 = pickNRandomIndices(len(lines), N)
    lines20, lines80 = splitLines(lines, indices20)
    writeFile(instance.file_names.rand_file, [str(index) for index in sorted(indices20)])
    writeFile(instance.file_names.output80_file, lines80)
    writeFile(instance.file_names.output20_file, lines20)

# Mode (1) from top comment
def performStoredSplit(instance):
    lines = readFile(instance.file_names.input_file)
    indices20 = [int(line) for line in readFile(instance.file_names.rand_file)]
    lines20, lines80 = splitLines(lines, indices20)
    writeFile(instance.file_names.output80_file, lines80)
    writeFile(instance.file_names.output20_file, lines20)

if __name__ == '__main__':
    for k,v in instances.items():
        #performFreshSplit(v)
        performStoredSplit(v)
