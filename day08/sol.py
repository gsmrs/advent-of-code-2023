import re
import itertools

pat = re.compile(r"(\w+) = \((\w+), (\w+)\)")
with open("input3.example.txt") as f:
# with open("input.txt") as f:
    insts = f.readline().strip()

    f.readline()

    paths = {}
    for line in f:
        m = pat.match(line)
        assert m
        paths[m.group(1)] = (m.group(2), m.group(3))

    print(paths)

starts = [key for key in paths.keys() if key.endswith("A")]
ends   = [key for key in paths.keys() if key.endswith("Z")]

print(starts)
print(ends)

current = starts[:]

counter = 0
last_seen = {}
for iteration, inst in enumerate(itertools.cycle(insts)):
    next_current = []
    counter += 1
    for node in current:
        if (inst, node) in last_seen:
            print("iteration =", iteration, "already visited; periodicity = ", iteration - last_seen[(inst, node)])
        else:
            last_seen[(inst, node)] = iteration

        left, right = paths[node]
        if inst == 'L':
            next_current.append(left)
        elif inst == 'R':
            next_current.append(right)
    current = next_current
    if all(node[2] == 'Z' for node in current):
        break

print(counter)
