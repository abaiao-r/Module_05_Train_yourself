# Bonus — Multi-Run Statistics (`--runs N`)

Run the simulation N times with different random seeds and aggregate travel-time statistics across all runs.

> Back to [README](../README.md) · See also [TECHNICAL.md](TECHNICAL.md)

---

## Usage

```bash
# Run 1000 simulations and display statistics
./bin/train_yourself network.txt trains.txt --runs 1000

# Combine with time-based routing
./bin/train_yourself network.txt trains.txt --time --runs 500

# Combine with animation (animation plays run 0 only)
./bin/train_yourself network.txt trains.txt --animate --runs 100

# Shortcut
make run-multi    # runs 1000 by default (editable in Makefile)
```

### CLI flag

```
--runs N    Run simulation N times and report average travel times
            N must be >= 1. Default is 1 (single run, no stats).
```

Validation: missing argument → error; N < 1 → error.

---

## How It Works

### Architecture

The multi-run logic lives entirely in `src/main.cpp`. There is no dedicated class — it wraps the existing single-run flow in a loop, collecting statistics after each iteration.

### Algorithm

```
for run = 0 to N-1:
    1. Re-create InputHandler::loadData()       ← fresh random seed each time
    2. Re-create DijkstraPathfinding + Simulation
    3. If run > 0: suppress console output (sim.setQuiet(true))
    4. sim.run()                                 ← events trigger randomly
    5. Collect per-train: actual time, total delay
    6. Graph export + animation only on run 0

After all runs:
    For each train: compute avg/min/max actual time, avg delay
    Print statistics table
```

### Data Structures

| Variable | Type | Purpose |
|----------|------|---------|
| `statActual` | `map<string, vector<double>>` | Per-train actual travel times across runs |
| `statDelay` | `map<string, vector<double>>` | Per-train total delay per run |
| `statEstimated` | `map<string, double>` | Per-train estimated (event-free) time (constant) |

### Statistics Computed

For each train, after all N runs:

| Metric | Formula |
|--------|---------|
| **Estimated time** | Stored once (no events, deterministic) |
| **Average actual time** | `sum(statActual[name]) / N` |
| **Min actual time** | Minimum of `statActual[name]` |
| **Max actual time** | Maximum of `statActual[name]` |
| **Average delay** | `sum(statDelay[name]) / N` |

All times formatted as `HHhMMmSSs`.

---

## Output

### First Run

The first run (run 0) behaves exactly like a normal single run:
- Per-train `.result` files are written
- Console output is printed normally
- Animation plays if `--animate` is also set
- Graph export happens if `--graph` is also set

### Subsequent Runs

Runs 1 through N-1 are **silent** — no console output, no file writes. Only statistics are collected.

### Statistics Table

After all runs complete, a summary table is printed:

```
=== Multi-Run Statistics (1000 runs) ===
------------------------------------------------------------------------
Train         Estimated     Avg Actual    Min Actual    Max Actual    Avg Delay
------------------------------------------------------------------------
TrainAB       01h02m30s     01h15m42s     01h05m10s     01h30m03s     00h13m12s
TrainCD       00h45m00s     00h52m18s     00h45m00s     01h12m45s     00h07m18s
------------------------------------------------------------------------
```

---

## Why This Is Useful

Because events are **probabilistic** (each event has a trigger probability between 0.0 and 1.0), a single run only shows one possible outcome. Running N times reveals:

- **Average-case** travel times (how long trains typically take)
- **Best-case** (minimum) — when no/few events trigger
- **Worst-case** (maximum) — when many events trigger simultaneously
- **Average delay** — the expected disruption impact per train

This turns the simulator from a single-point estimate into a **statistical analysis tool**.
