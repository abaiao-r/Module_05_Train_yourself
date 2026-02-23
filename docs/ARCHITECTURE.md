# Architecture

> Back to [TECHNICAL.md](TECHNICAL.md) · [README](../README.md)

---

## Overview

The simulator follows a layered architecture with clear separation of concerns:

```
main.cpp
  └─ InputHandler          (parse files → SimulationData)
  └─ Simulation            (orchestrator)
       ├─ RailNetwork      (graph: nodes + edges)
       ├─ Train[]          (state machines with physics properties)
       ├─ Event[]          (probabilistic disruptions)
       ├─ TrainState[]     (runtime state per train: position, speed, timers)
       ├─ IPathfinding     (strategy interface → DijkstraPathfinding)
       ├─ ISimulationObserver[] (observer interface → FileOutputObserver)
       └─ OutputManager    (formatted console output)
  └─ GraphExporter         (--graph: Graphviz DOT export)
  └─ TerminalAnimDisplay   (--animate: ANSI terminal renderer)
```

---

## Folder Structure

Each class lives in its own folder with separated `.hpp` / `.cpp` files:

```
src/
├── main.cpp                              Entry point (--help flag)
├── Node/                                 Station in the network
├── Edge/                                 Connection between nodes (header-only struct)
├── RailNetwork/                          Graph: nodes + adjacency list
├── Train/                                Train with physics properties and state
├── Event/                                Probabilistic disruption
├── TrainFactory/                         Factory for validated Train creation (9 params)
├── IPathfinding/                         Strategy interface (header-only)
├── DijkstraPathfinding/                  Shortest-path algorithm
├── InputHandler/                         File parser → SimulationData
├── OutputManager/                        Formatted console output
├── ISimulationObserver/                  Observer interface (header-only)
├── FileOutputObserver/                   Per-train .result file writer
├── GraphExporter/                        Graphviz DOT export (--graph flag)
├── TerminalAnimDisplay/                  Terminal animation (--animate bonus)
└── Simulation/                           Orchestrator: physics engine, concurrent sim
bonus/gui/
├── main.cpp                              Qt application entry point
├── MainWindow.cpp/hpp                    Full GUI: menus, CRUD, toolbar, simulation
├── NetworkScene.cpp/hpp                  Interactive graph visualisation (drag, layout)
├── SimulationWorker.cpp/hpp              Background-threaded simulation runner
├── SimDashboard.cpp/hpp                  Live HTML dashboard widget
└── bonus.pro                             qmake project file
scripts/
└── check_deps.sh                         Cross-platform dependency checker/installer
tests/
├── TestFramework.hpp                     Custom assertion macros + test runner
├── NodeTest.cpp                          5 tests
├── RailNetworkTest.cpp                   14 tests
├── TrainTest.cpp                         14 tests (including physics)
├── EventTest.cpp                         13 tests
├── DijkstraTest.cpp                      11 tests
├── InputHandlerTest.cpp                  28 tests (including validation & rail events)
├── TrainFactoryTest.cpp                  11 tests (including weight/friction/stop/ID validation)
├── OutputManagerTest.cpp                 5 tests
├── IntegrationTest.cpp                   7 tests  (end-to-end with .result file checks)
├── EdgeCaseTest.cpp                      9 tests  (boundary values & error handling)
└── CombinationTest.cpp                   198 tests (exhaustive parameter combinations)
```

**Runtime output directories** (created automatically):

```
output/
├── results/         Per-train .result files
└── graphs/          --graph DOT/PNG/SVG exports (default when no path separator in filename)
```

---

## Data Flow

```
network.txt ─┐
              ├─ InputHandler::loadData() ─→ SimulationData
trains.txt  ─┘                                    │
                                                   ▼
                                            Simulation(network, trains,
                                                       events, pathfinder)
                                                   │
                                              sim.run()
                                                   │
                              ┌─────────────────────┼───────────────────────┐
                              ▼                     ▼                       ▼
                        computePaths      concurrent physics loop     printResult
                        (Dijkstra)       ┌──────────────────────┐    (console summary)
                                         │  for each second:    │
                                         │  • check departures  │
                                         │  • handle stops      │
                                         │  • updatePhysics()   │
                                         │  • transitions       │
                                         │  • observer output   │
                                         │  • applyBlocking()   │
                                         └──────────────────────┘
                                                   │
                                    ┌──────────────┼──────────────┐
                                    ▼              ▼              ▼
                           TrainAB.result   TrainAC.result   TrainBA.result
```

---

## Dependency Injection

`Simulation` receives all its dependencies through the constructor — `RailNetwork`, `Train`s, `Event`s, and `IPathfinding` — enabling unit-testable design without globals or singletons.

See [DESIGN_PATTERNS.md · Dependency Injection](DESIGN_PATTERNS.md#dependency-injection) for the full constructor signature.

---

## Orthodox Canonical Form

All value-type classes (`Node`, `Train`, `Event`, `RailNetwork`, `OutputManager`, `DijkstraPathfinding`) implement the canonical four: parameterised constructor, copy constructor, copy assignment operator, destructor. `RailNetwork` additionally provides move constructor and move assignment. `Simulation`, `FileOutputObserver`, and `TerminalAnimDisplay` explicitly delete copy operations.

See [DESIGN_PATTERNS.md · Orthodox Canonical Form](DESIGN_PATTERNS.md#orthodox-canonical-form) for the full table.
