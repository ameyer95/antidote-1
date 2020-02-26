# Output 100 random indices to use from the full (but already 1/7-restricted) mnist test set

import random

random.seed(0)

s = set()
while len(s) < 100:
    s.add(random.randrange(2164))

indices = sorted(list(s))
for i in indices:
    print(i)
