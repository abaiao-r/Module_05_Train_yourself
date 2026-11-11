#!/usr/bin/env python3
"""Writes input/video/: one invented city that grows one idea per level.

usage: gen_video_scenarios.py [repoRoot]   (default: the parent of scripts/)
"""
import os, random, sys

ROOT = sys.argv[1] if len(sys.argv) > 1 else os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")
OUT = os.path.join(ROOT, "input", "video")
os.makedirs(OUT, exist_ok=True)

# Fixed positions shared by every level (hand placed, nothing overlaps).
POS = {
    "Harbor": (0, 500), "Old_Town": (600, 620), "Ridge": (600, 140),
    "Central": (1200, 500), "Airport": (1800, 200), "Campus": (1800, 800),
    "Stadium": (1200, 1050), "Market": (600, 1050), "Beach": (0, 1050),
    "Hills": (1200, -50),
}

SCALE = 0.23  # small coordinates = large labels once fitted

# Train presets: name weight friction accel brake
COMMUTER = "200 0.05 300.0 50.0"
EXPRESS = "200 0.05 360.0 60.0"
FREIGHT = "1000 0.05 520.0 400.0"

def write(name, nodes, rails, trains, events=()):
    with open(os.path.join(OUT, f"railNetwork{name}.txt"), "w") as f:
        for n in nodes:
            f.write(f"Node {n}\n")
        for r in rails:
            f.write("Rail " + " ".join(str(x) for x in r) + "\n")
        for e in events:
            f.write("Event " + e + "\n")
    with open(os.path.join(OUT, f"railNetwork{name}.layout"), "w") as f:
        f.write("# Station x y (video city)\n")
        for n in nodes:
            f.write(f"{n} {POS[n][0] * SCALE:g} {POS[n][1] * SCALE:g}\n")
    with open(os.path.join(OUT, f"trainPrint{name}.txt"), "w") as f:
        for t in trains:
            f.write(t + "\n")

# L1 physics: one train, one rail.
write("L1Physics", ["Harbor", "Old_Town"],
      [("Harbor", "Old_Town", 8.0, 160.0, 1)],
      [f"Blue {COMMUTER} Harbor Old_Town 08h00 00h02"])

# L2 routing: short but slow through Old Town vs long but fast over the Ridge.
L2_NODES = ["Harbor", "Old_Town", "Ridge", "Central"]
def l2_rails(lines=1):
    return [("Harbor", "Old_Town", 8.0, 60.0, 1),
            ("Old_Town", "Central", 8.0, 60.0, 1),
            ("Harbor", "Ridge", 12.0, 200.0, lines),
            ("Ridge", "Central", 12.0, 200.0, lines)]
write("L2Route", L2_NODES, l2_rails(),
      [f"Blue {COMMUTER} Harbor Central 08h00 00h02"])

# L3 head-on: a second train comes the other way on the single line, then 2 lines.
HEADON = [f"Blue {COMMUTER} Harbor Central 08h00 00h02",
          f"Red {COMMUTER} Central Harbor 08h06 00h02"]
write("L3HeadOn", L2_NODES, l2_rails(1), HEADON)
write("L3TwoLines", L2_NODES, l2_rails(2), HEADON)

# L4 overtake: a slow freight leaves first, the express catches up.
FOLLOW = [f"Freight {FREIGHT} Harbor Central 08h00 00h02",
          f"Express {EXPRESS} Harbor Central 08h02 00h02"]
write("L4Stuck", L2_NODES, l2_rails(1), FOLLOW)
write("L4Overtake", L2_NODES, l2_rails(2), FOLLOW)

# L5 event: the Ridge bridge closes for 30 min.
write("L5Bridge", L2_NODES, l2_rails(2),
      [f"Blue {COMMUTER} Harbor Central 08h00 00h02"],
      ["Bridge_closed 1.0 30m Ridge Central"])

# L6 rush hour: the city grows; everybody crosses Central.
L6_NODES = ["Harbor", "Old_Town", "Ridge", "Central", "Airport", "Campus",
            "Stadium", "Market", "Beach", "Hills"]
L6_RAILS = l2_rails(1) + [
    ("Central", "Airport", 14.0, 160.0, 2),
    ("Central", "Campus", 14.0, 120.0, 1),
    ("Airport", "Campus", 14.0, 120.0, 1),
    ("Central", "Stadium", 10.0, 120.0, 1),
    ("Stadium", "Campus", 14.0, 100.0, 1),
    ("Harbor", "Beach", 10.0, 100.0, 1),
    ("Beach", "Market", 12.0, 100.0, 1),
    ("Market", "Stadium", 12.0, 120.0, 1),
    ("Old_Town", "Market", 9.0, 80.0, 1),
    ("Ridge", "Hills", 12.0, 120.0, 1),
    ("Hills", "Airport", 14.0, 160.0, 1),
]
L6_EVENTS = ["Signal_fault 0.3 15m Central Airport",
             "Crowd 0.3 10m Central"]
west = ["Harbor", "Beach", "Old_Town"]
east = ["Airport", "Campus", "Stadium"]
rng = random.Random(7)
trains = []
for i in range(30):
    a, b = rng.choice(west), rng.choice(east)
    if i % 2:
        a, b = b, a
    preset = rng.choice([COMMUTER, COMMUTER, EXPRESS])
    trains.append(f"T{i + 1:02d} {preset} {a} {b} 08h{(i // 3) * 2:02d} 00h02")
write("L6RushHour", L6_NODES, L6_RAILS, trains, L6_EVENTS)
print("wrote", OUT)
