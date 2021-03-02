import random

random.seed(0)

s = set()
while len(s) < 100:
    s.add(random.randrange(2164))

indices = sorted(list(s))
for i in indices:
    print(i)
