# Technical Documentation

## Architecture Overview

The simulator follows a layered architecture with clear separation of concerns:

```
main.cpp
  └─ InputHandler        (parse files → SimulationData)
  └─ Simulation          (orchestrator)
       ├─ RailNetwork    (graph: nodes + edges)
       ├─ Train[]        (state machines)
       ├─ Event[]        (probabilistic disruptions)
       ├─ IPathfinding   (strategy interface → DijkstraPathfinding)
       └─ OutputManager  (formatted output)
```

## Folder Structure

Each class lives in its own folder with separated `.hpp` / `.cpp` files:

```
src/
├── main.cpp                              Entry point
├── Node/                                 Station in the network
├── Edge/                                 Connection between nodes (header-only struct)
├── RailNetwork/                          Graph: nodes + adjacency list
├── Train/                                Train state and path tracking
├── Event/                                Probabilistic disruption
├── TrainFactory/                         Factory for validated Train creation
├── IPathfinding/                         Strategy interface (header-only)
├── DijkstraPathfinding/                  Shortest-path algorithm
├── InputHandler/                         File parser → SimulationData
├── OutputManager/                        Formatted console output
└── Simulation/                           Orchestrator: paths, simulation loop, events
tests/
├── TestFramework.hpp                     Custom assertion macros + test runner
├── NodeTest.cpp                          5 tests
├── RailNetworkTest.cpp                   14 tests
├── TrainTest.cpp                         11 tests
├── EventTest.cpp                         10 tests
├── DijkstraTest.cpp                      7 tests
├── InputHandlerTest.cpp                  8 tests
├── TrainFactoryTest.cpp                  8 tests
├── OutputManagerTest.cpp                 5 tests
└── IntegrationTest.cpp                   5 tests  (end-to-end)
```

## Design Patterns

### Strategy — Pathfinding

`IPathfinding` defines the interface; `DijkstraPathfinding` is the concrete strategy. The algorithm is injected into `Simulation` at construction, making it trivial to swap (e.g., A\*, BFS):

```cpp
class IPathfinding {
public:
    virtual ~IPathfinding() = default;
    virtual std::vector<std::shared_ptr<Node>> findPath(
        const std::string &start, const std::string &end,
        const RailNetwork &network) const = 0;
};
```

### Factory — Train Creation

`TrainFactory::createTrain()` validates all parameters before constructing a `Train`. Invalid input (empty name, zero acceleration, negative departure, etc.) throws `std::invalid_argument`.

### Dependency Injection

`Simulation` receives all its dependencies through the constructor — `RailNetwork`, `Train`s, `Event`s, and `IPathfinding` — enabling unit-testable design without globals or singletons.

### Orthodox Canonical Form

All value-type classes (`Node`, `Train`, `Event`, `RailNetwork`, `OutputManager`, `DijkstraPathfinding`) implement the canonical four: default/parameterised constructor, copy constructor, copy assignment operator, destructor. `RailNetwork` additionally provides move constructor and move assignment.

## Class Reference

### Edge (struct)

Header-only POD in `src/Edge/Edge.hpp`:

| Member | Type | Description |
|---|---|---|
| `destination` | `shared_ptr<Node>` | Target node |
| `distance` | `double` | Kilometres |
| `speedLimit` | `double` | km/h |

### Node

A station in the rail network. Throws `InvalidNameException` on empty name.

### RailNetwork

Undirected graph stored as `unordered_map<string, shared_ptr<Node>>` + `unordered_map<string, vector<Edge>>`. Validates: no self-loops, positive distance/speed, no duplicates. Custom exceptions: `NodeNotFoundException`, `DuplicateNodeException`, `DuplicateConnectionException`.

### Train

Tracks a train's journey: path (sequence of `Node`s), current index, status (`Waiting` → `Running` → `Arrived`/`Delayed`), accumulated delay. `advanceToNextNode(travelTime)` progresses the train; `applyDelay(seconds)` adds disruption time.

### Event

Probabilistic disruption bound to a node. `tryTrigger(rng)` rolls against `_probability` using a uniform distribution. Duration is in seconds.

### InputHandler

Static utility. Parses two text files into a `SimulationData` struct. Supports quoted event names (`"Passenger's Discomfort"`), duration units (`30m`, `48h`, `356d`), and time format (`14h10`).

### Simulation

Orchestrator. Flow:
1. Sort trains by departure time
2. Compute shortest paths via injected `IPathfinding`
3. For each train: simulate step-by-step, compute travel time = `(distance / speedLimit) × 3600`, trigger events probabilistically at each node
4. Print results

### OutputManager

Stateless formatter. `formatTime(double seconds)` converts to `HH:MM:SS`. All output goes to `std::cout`.

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
                                    ┌──────────────┼──────────────┐
                                    ▼              ▼              ▼
                              computePaths   simulateTrain   printResult
                              (Dijkstra)    (step + events)  (summary)
```

## Build System

Plain `Makefile` with:
- **No relink**: pattern rules produce `.o` in `objs/`, binary only re-links if objects change
- **Compiler**: `CXX = c++` (overridable: `make CXX=g++`)
- **Flags**: `-Wall -Wextra -Werror -std=c++17 -g`
- **Targets**: `all`, `clean`, `fclean`, `re`, `test`, `run`

## Testing

Custom `TestFramework.hpp` providing:
- `ASSERT_TRUE`, `ASSERT_FALSE`, `ASSERT_EQ`, `ASSERT_STR_EQ`, `ASSERT_THROWS`
- Coloured terminal output (ANSI)
- Per-suite summary with pass/fail count

Each test binary is a standalone executable returning 0 (all pass) or 1 (any failure), compatible with CI and valgrind.

## CI Pipeline

GitHub Actions on every pull request to `master`/`main`:
- **Matrix**: GCC + Clang
- **Steps**: build → `make test` → valgrind on every binary (leak-check=full, error-exitcode=1)
