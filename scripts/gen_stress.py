#!/usr/bin/env python3
"""Deterministic stress input generator for the --lines bonus.

usage: gen_stress.py [seed] [cols] [rows] [trains]
Writes input/railNetworkPrintFolder/railNetworkBonus13Stress.txt and
input/trainPrintFolder/trainPrintBonus13Stress.txt.
"""
import random
import sys

seed = int(sys.argv[1]) if len(sys.argv) > 1 else 42
cols = int(sys.argv[2]) if len(sys.argv) > 2 else 20
rows = int(sys.argv[3]) if len(sys.argv) > 3 else 15
n_trains = int(sys.argv[4]) if len(sys.argv) > 4 else 3000
rnd = random.Random(seed)

name = lambda c, r: f"S{c:02d}_{r:02d}"
nodes = [name(c, r) for r in range(rows) for c in range(cols)]
pairs = set()
for r in range(rows):
    for c in range(cols):
        if c + 1 < cols:
            pairs.add((name(c, r), name(c + 1, r)))
        if r + 1 < rows:
            pairs.add((name(c, r), name(c, r + 1)))
# a few long express chords between random far-apart nodes
while len(pairs) < int(len(nodes) * 2) - cols - rows + 30:
    a, b = rnd.sample(nodes, 2)
    if a != b and (a, b) not in pairs and (b, a) not in pairs:
        pairs.add((a, b))

out = [f"Node {n}" for n in nodes]
pairs = sorted(pairs)
lines_of = {p: rnd.choice([1, 1, 2, 2, 3, 4]) for p in pairs}

# events: city events, rail events (edge-wide) and single-line events
for i in range(12):
    out.append(f"Event Breakdown{i} {rnd.choice([0.05, 0.1, 0.2, 0.3])} "
               f"{rnd.choice([5, 10, 20, 45])}m {rnd.choice(nodes)}")
rail_events = rnd.sample(pairs, 40)
for i, (a, b) in enumerate(rail_events):
    line = f" {rnd.randint(1, lines_of[(a, b)])}" if i % 2 else ""
    out.append(f"Event Fault{i} {rnd.choice([0.1, 0.2, 0.4])} "
               f"{rnd.choice([10, 15, 30])}m {a} {b}{line}")

for a, b in pairs:
    n = lines_of[(a, b)]
    base = rnd.choice([80.0, 120.0, 160.0, 200.0])
    dist = round(rnd.uniform(0.8, 6.0), 1)
    speeds = " ".join(str(float(rnd.choice([40, 60, 80, 120, 160, 200])))
                      for _ in range(n))
    out.append(f"Rail {a} {b} {dist} {base} {n} {speeds}")
open("input/railNetworkPrintFolder/railNetworkBonus13Stress.txt",
     "w", encoding="utf-8").write("\n".join(out) + "\n")

trains = []
for i in range(n_trains):
    a, b = rnd.sample(nodes, 2)
    t = rnd.randint(5 * 60, 20 * 60 + 59)
    trains.append(
        f"T{i:04d} {rnd.choice([80, 120, 200, 300, 450])} "
        f"{rnd.choice([0.02, 0.04, 0.05])} "
        f"{rnd.choice([300.0, 400.0, 500.0, 600.0])} "
        f"{rnd.choice([40.0, 50.0, 80.0])} {a} {b} "
        f"{t // 60:02d}h{t % 60:02d} 00h0{rnd.randint(0, 3)}")
open("input/trainPrintFolder/trainPrintBonus13Stress.txt",
     "w", encoding="utf-8").write("\n".join(trains) + "\n")
print(len(nodes), "nodes,", len(pairs), "rails,", n_trains, "trains")
