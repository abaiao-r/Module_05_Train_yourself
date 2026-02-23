# Train Yourself — Rail Network Simulator

A physics-based train simulation engine that models rail traffic across a configurable network of stations and tracks. Trains find optimal routes via Dijkstra, accelerate and brake according to real forces (F = ma with rolling friction), handle overtaking on shared segments, and produce per-train result files with every-minute snapshots.

> **42 OOP Piscine — Module 05**

---

## Features

### Mandatory

| Feature | Detail |
|---------|--------|
| **Physics engine** | 1-second discrete timestep · acceleration = `(F_accel − μmg) / m` · braking distance = `v² / (2·decel)` |
| **Concurrent simulation** | All trains run on a shared wall clock; overtaking caps a trailing train's speed |
| **Pathfinding** | Dijkstra shortest-path (Strategy pattern) · distance or time weight mode (`--time` flag) |
| **Per-train output** | `TrainName_HHhMM.result` file with header, estimated time, every-minute rail graph, events, actual time |
| **Random events** | Probability-based disruptions at stations (riots, discomfort, …) inject delays |
| **315 unit / integration tests** | Custom TestFramework, 11 suites covering nodes, networks, trains, events, pathfinding, I/O, factories, edge cases, combinations |
| **3 design patterns + 6 principles** | Strategy, Factory, Observer · KISS, DRY, Encapsulation, SRP, Separation of Concerns, Interface Segregation |
| **7 UML diagrams** | Class, 3 sequence, state machine, 2 activity — PlantUML sources + PNG + SVG |

### Bonus Features

| Bonus | Description | Docs |
|-------|-------------|------|
| **Multi-Run Statistics** | `--runs N` — run the simulation N times with different random seeds and report average/min/max travel times and delays | [docs/BONUS_MULTIRUN.md](docs/BONUS_MULTIRUN.md) |
| **Terminal Animation** | `--animate` — live full-screen terminal dashboard with Unicode box drawing, progress bars, coloured status indicators, ~20 FPS | [docs/BONUS_ANIMATE.md](docs/BONUS_ANIMATE.md) |
| **Qt GUI** | Interactive graph editor with full CRUD, drag-and-drop nodes, force-directed layout, live simulation with animated train dots, real-time dashboard, speed control, preset library | [docs/BONUS_GUI.md](docs/BONUS_GUI.md) |

---

## Quick Start

### Requirements

- A C++17 compiler (`c++`, `g++`, or `clang++`)
- `make`
- **Qt 6** (only for the GUI bonus)
- Supported platforms: **macOS**, **Linux**, **WSL**

### Install Dependencies

The project can check and install everything it needs automatically:

```bash
make deps           # check & install CLI build dependencies
make deps-gui       # check & install CLI + Qt 6 for the GUI bonus
make check-deps     # just check CLI deps (no install)
make check-deps-gui # just check CLI + Qt deps (no install)
```

<details>
<summary>Manual install per platform</summary>

**macOS**
```bash
xcode-select --install   # C++ compiler & make
brew install qt           # Qt 6 (GUI bonus only)
```

**Ubuntu / Debian / WSL**
```bash
sudo apt-get update && sudo apt-get install -y build-essential   # C++ & make
sudo apt-get install -y qt6-base-dev                              # Qt 6 (GUI bonus only)
```

**Fedora**
```bash
sudo dnf install -y gcc-c++ make       # C++ & make
sudo dnf install -y qt6-qtbase-devel   # Qt 6 (GUI bonus only)
```

**Arch Linux**
```bash
sudo pacman -S --noconfirm base-devel   # C++ & make
sudo pacman -S --noconfirm qt6-base     # Qt 6 (GUI bonus only)
```
</details>

### Build & Run

```bash
make                # build
make run            # run with sample data
./bin/Train --help  # show input file format

# Or supply your own files
./bin/Train path/to/network.txt path/to/trains.txt

# Optimise route by travel time instead of distance
./bin/Train path/to/network.txt path/to/trains.txt --time

# Terminal animation
make run-animate

# Multi-run statistics
make run-multi

# Graph export (Graphviz DOT + auto-render PNG/SVG)
./bin/Train network.txt trains.txt --graph output/network.dot
make run-graph    # shortcut with sample data

# GUI bonus
make bonus          # build (auto-checks Qt 6)
make run-gui        # build & launch
```

### CLI Reference

<details>
<summary><code>./bin/Train --help</code> (click to expand)</summary>

```
Usage: ./Train <network_file> <train_file> [options]

Options:
  --time              Optimise route by travel time instead of distance
  --graph <file.dot>  Export network + paths as Graphviz DOT file
  --animate           Show live terminal animation of the simulation
  --runs N            Run simulation N times and report average travel times

=== Network file format (.txt) ===
  Node <name>              Declares a station/junction
  Rail <from> <to> <km> <kmh>  Bidirectional track
  Event <name> <prob> <dur> <node> [node2]  Random disruption

=== Train file format (.txt) ===
  <name> <weight_t> <friction> <accel_kN> <brake_kN>
         <from> <to> <departure_time> <stop_duration>

=== Output ===
  One .result file per train in output/results/
```

</details>

### Graph Export (`--graph`)

The `--graph` flag exports the rail network and all computed train paths as a **Graphviz DOT** file:

```bash
./bin/Train network.txt trains.txt --graph output/network.dot
```

- Stations are drawn as rounded boxes (city nodes highlighted in blue)
- Edges show distance (km) and speed limit (km/h)
- Each train's computed path is overlaid in a unique colour (red, green, blue, orange, purple, cyan, magenta, lime)
- If Graphviz is installed (`dot` command), PNG and SVG are auto-rendered
- Dark theme background matching the GUI aesthetic

### Run Tests

```bash
make test     # 315 tests across 11 suites
```

### Clean

```bash
make clean       # remove object files
make fclean      # remove object files and binaries
make bonus-clean # remove GUI build artifacts
make re          # full rebuild
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

See [docs/TECHNICAL.md](docs/TECHNICAL.md) for the full documentation index, or jump directly to:

| Document | Content |
|----------|---------|
| [Business Logic](docs/BUSINESS_LOGIC.md) | All 15 business rules (core + bonus) |
| [Architecture](docs/ARCHITECTURE.md) | Layered design, folder structure, data flow |
| [Design Patterns](docs/DESIGN_PATTERNS.md) | Strategy, Factory, Observer · DI · Class Relationships · 6 Design Principles |
| [Class Reference](docs/CLASS_REFERENCE.md) | Per-class API documentation |
| [Diagrams](docs/DIAGRAMS.md) | 7 UML diagrams (class, state, sequence, activity) |
| [Build & Testing](docs/BUILD_SYSTEM.md) | Makefile targets, 315 tests, CI pipeline |

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

### Bonus Documentation

| Document | Content |
|----------|---------|
| [docs/BONUS_MULTIRUN.md](docs/BONUS_MULTIRUN.md) | Multi-run statistics architecture, output format, CLI usage |
| [docs/BONUS_ANIMATE.md](docs/BONUS_ANIMATE.md) | Terminal animation rendering, ANSI codes, frame layout |
| [docs/BONUS_GUI.md](docs/BONUS_GUI.md) | Qt GUI architecture, CRUD, graph editor, simulation runner, dashboard |

## Author

**André Francisco** ([abaiao-r](https://github.com/abaiao-r)) — 42 Lisboa

## License

This project is licensed under the MIT License — see [LICENSE](LICENSE) for details.