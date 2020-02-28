import random
from functools import reduce

# Creates a random line in 100x100x{foo,bar}
def random_line():
    return random.randrange(100), random.randrange(100), ("foo" if random.random() > 0.5 else "bar")

def write_random_lines(filename, num_lines):
    f = open(filename, 'w')
    for i in range(num_lines):
        line = [str(x) for x in random_line()]
        f.write(reduce(lambda x,y : x + ',' + y, line) + "\n")
    f.close()

if __name__ == '__main__':
    random.seed(1)
    write_random_lines("foo-train.csv", 30)
    write_random_lines("foo-test.csv", 10)
