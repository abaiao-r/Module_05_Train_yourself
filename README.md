# Train Yourself — Rail Network Simulator

A physics-based train simulation engine that models rail traffic across a configurable network of stations and tracks. Trains find optimal routes via Dijkstra, accelerate and brake according to real forces (F = ma with rolling friction), handle overtaking on shared segments, and produce per-train result files with every-minute snapshots.

> **42 OOP Piscine — Module 05**

## Features

| Feature | Detail |
|---------|--------|
| **Physics engine** | 1-second discrete timestep · acceleration = `(F_accel − μmg) / m` · braking distance = `v² / (2·decel)` |
| **Concurrent simulation** | All trains run on a shared wall clock; overtaking caps a trailing train's speed |
| **Pathfinding** | Dijkstra shortest-path (Strategy pattern) · distance or time weight mode (`--time` flag) |
| **Per-train output** | `TrainName_HHhMM.result` file with header, estimated time, every-minute rail graph, events, actual time |
| **Random events** | Probability-based disruptions at stations (riots, discomfort, …) inject delays |
| **86 unit / integration tests** | Custom TestFramework, 9 suites, CI with gcc + clang matrix |
| **3 design patterns** | Strategy (pathfinding), Factory (train creation), Observer (file output) |
| **7 UML diagrams** | Class, 3 sequence, state machine, 2 activity — PlantUML sources + PNGs |

## Quick Start

### Requirements

- A C++17 compiler (`c++`, `g++`, or `clang++`)
- `make`

### Build & Run

```bash
make                # build
make run            # run with sample data
./bin/Train --help  # show input file format

# Or supply your own files
./bin/Train path/to/network.txt path/to/trains.txt

# Optimise route by travel time instead of distance
./bin/Train path/to/network.txt path/to/trains.txt --time
```

### Run Tests

```bash
make test     # 86 tests across 9 suites
```

### Clean

```bash
make clean    # remove object files
make fclean   # remove object files and binaries
make re       # full rebuild
```

## Input Format

The simulator reads two text files (pass `--help` for full details).

### Network file

Defines stations, events, and rail connections:

```
Node CityA
Node CityB
Event Riot 0.05 48h CityA
Event "Passenger's Discomfort" 0.2 30m CityB
Rail CityA CityB 15.0 250.0
```

Each `Rail` line: `from`, `to`, `distance (km)`, `speed limit (km/h)`.

### Train file

Defines trains with physics properties:

```
TrainAB 80 0.05 356.0 30.0 CityA CityB 14h10 00h10
```

| # | Field | Unit |
|---|-------|------|
| 1 | Name | — |
| 2 | Weight | metric tons |
| 3 | Friction coefficient | — |
| 4 | Max acceleration force | kN |
| 5 | Max brake force | kN |
| 6 | Departure station | — |
| 7 | Arrival station | — |
| 8 | Departure time | `HHhMM` |
| 9 | Stop duration at stations | `HHhMM` |

## Output

Each train produces a `.result` file:

```
Train : TrainAB
Final travel time : 00h32m

[00h00] - [    CityA][RailNodeA] - [53.00km] - [Speed up] - [x][ ][ ] ... [ ][ ]
[00h01] - [    CityA][RailNodeA] - [51.20km] - [Speed up] - [x][ ][ ] ... [ ][ ]
...
[00h32] - [RailNodeC][    CityB] - [00.00km] - [ Stopped] - [ ][ ][ ][ ][x]

Actual travel time : 00h32m
```

Rail graph: `[x]` = this train, `[O]` = blocking train, `[ ]` = empty (1 cell per km).

## Architecture

See [docs/TECHNICAL.md](docs/TECHNICAL.md) for the full breakdown.

### Design Patterns

- **Strategy** — `IPathfinding` → `DijkstraPathfinding` (swap algorithm without touching simulation)
- **Factory** — `TrainFactory::createTrain()` validates all 9 fields before construction
- **Observer** — `ISimulationObserver` → `FileOutputObserver` (decouple simulation from I/O)

### Diagrams

All diagrams live in `docs/diagrams/` (`.puml` sources + `.png` renders):

| Diagram | Description |
|---------|-------------|
| Class diagram | Full project structure with all classes and relationships |
| Sequence — Simulation run | Concurrent physics loop with observer notifications |
| Sequence — Input parsing | 9-field parsing flow through InputHandler → TrainFactory |
| Sequence — Pathfinding | Dijkstra algorithm interaction with RailNetwork graph |
| State — Train lifecycle | Waiting → Running → Stopped / Delayed → Arrived |
| Activity — Physics tick | Per-second brake / accelerate / maintain decision |
| Activity — Simulation overview | End-to-end flow from file loading to result generation |

## Author

**André Francisco** ([abaiao-r](https://github.com/abaiao-r)) — 42 Lisboa

## License

This project is licensed under the MIT License — see [LICENSE](LICENSE) for details.