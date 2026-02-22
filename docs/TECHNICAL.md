# Technical Documentation

## Table of Contents

1. [Business Logic](#business-logic)
2. [Class Diagram](#class-diagram)
3. [Sequence Diagrams](#sequence-diagrams)
4. [Architecture Overview](#architecture-overview)
5. [Design Patterns](#design-patterns)
6. [Build System](#build-system)
7. [Testing](#testing)
8. [CI Pipeline](#ci-pipeline)

---

## Business Logic

### What the Simulator Models

The system models a **rail network** where trains travel between stations along tracks using realistic physics, finding the fastest routes and encountering random disruptions along the way. All trains are simulated **concurrently** with a shared clock. It answers: *"Given a network, train specifications, and a schedule, when does each train arrive, and how do physics, stops, and disruptions affect travel time?"*

### Core Business Rules

#### 1. Network Definition

A rail network consists of **stations** (nodes) connected by **tracks** (edges). Each track has:
- A **distance** in kilometres (must be positive)
- A **speed limit** in km/h (must be positive)

Connections are **bidirectional** — if CityA connects to CityB, CityB also connects to CityA. Self-loops (a station connected to itself) are forbidden. Duplicate connections between the same pair of stations are rejected.

#### 2. Train Specification

Each train has:
- A **name** (unique identifier)
- A **unique numeric ID** (auto-assigned by the factory, starting at 1)
- **Weight** in metric tons (must be positive)
- **Friction coefficient** μ (must be non-negative)
- **Maximum acceleration force** in kilonewtons (must be positive)
- **Maximum braking force** in kilonewtons (must be positive)
- A **departure station** and an **arrival station** (must be different)
- A **departure time** (e.g., 14h10 = 2:10 PM)
- A **stop duration** at each intermediate station (e.g., 00h10 = 10 minutes)

Input format: `name weight friction accelForce brakeForce from to time stopDuration`

#### 3. Route Selection

Before simulation begins, each train is assigned an **optimal route** through the network using Dijkstra's algorithm. The cost metric is configurable via `PathWeightMode`:

- **Distance** (default) — minimises total kilometres: edge weight = `distance`
- **Time** — minimises travel time: edge weight = `distance / speedLimit`

Pass `--time` on the command line to use time-based routing. If no route exists, the train is skipped with a warning.

#### 4. Physics-Based Travel Time

Travel time is computed using a **discrete physics simulation** with 1-second timesteps. At each tick, the engine computes forces and updates position:

**Acceleration rate** (m/s²):

$$a_{accel} = \frac{F_{accel} \times 1000 - \mu \cdot m \times 1000 \cdot g}{m \times 1000}$$

**Deceleration rate** (m/s²):

$$a_{decel} = \frac{F_{brake} \times 1000 + \mu \cdot m \times 1000 \cdot g}{m \times 1000}$$

Where $F$ is in kN, $m$ is in metric tons, $g = 9.81$ m/s², and $\mu$ is the friction coefficient.

Each tick, the train decides:
- **Speed up** — if current speed < segment speed limit, apply $a_{accel}$
- **Brake** — if remaining distance ≤ braking distance ($d_{brake} = \frac{v^2}{2 \cdot a_{decel}}$), apply $-a_{decel}$
- **Maintain** — otherwise hold current speed

Speed is capped at the segment's speed limit. Position advances by $v \cdot \Delta t$ each second.

#### 5. Station Stops

When a train reaches an intermediate node (not the final destination), it stops for the configured **stop duration**. During stops, the train remains at position 0 on the next segment with speed 0. Output lines during stops show action `Stopped`.

#### 6. Concurrent Multi-Train Simulation

All trains are simulated **simultaneously** on a shared wall clock. Each train has its own `TrainState` tracking: segment index, position on segment (metres), speed (m/s), time since departure, stop timer, and departed/arrived flags.

#### 7. Overtaking / Blocking

When two trains occupy the **same segment** in the same direction, the train behind has its speed capped to the speed of the train ahead. In the output rail graph, blocking trains appear as `[O]` cells.

#### 8. Random Events (Disruptions)

Events can occur at **specific stations** (node events) or on **rail segments** between two stations (rail events). Each event has:
- A **probability** (0.0 to 1.0) of triggering
- A **duration** (the delay in seconds if triggered)

**Node events** trigger when a train arrives at the bound station. **Rail events** trigger when a train enters the bound segment (bidirectional match). Input format:

- Node event: `Event <name> <probability> <duration> <node_name>`
- Rail event: `Event <name> <probability> <duration> <node_name1> <node_name2>`

When triggered, the train's clock advances by the event's duration, and the delay is accumulated.

#### 9. Per-Train Output Files (Observer Pattern)

Each train produces a result file named `TrainName_DepartureTime.result` containing:
- **Header**: train name + ID + ID + estimated travel time
- **Timestep lines** (every 60 seconds): `[time] - [from][to] - [dist remaining] - [action] - [rail graph]`
- **Rail graph**: 1 cell per km — `[x]` = this train's position, `[O]` = blocking train, `[ ]` = empty
- **Event notifications** when triggered
- **Footer**: actual travel time

#### 10. Train State Machine

| State | Meaning |
|---|---|
| **Waiting** | Created, not yet departed |
| **Running** | Departed, travelling along route |
| **Delayed** | An event added delay to this train |
| **Arrived** | Reached the arrival station |

#### 11. Console Output

The simulator also produces console output:
1. **Network map** — all stations and their connections
2. **Route assignments** — the shortest path computed for each train
3. **Simulation log** — timestamped departures, arrivals, and events
4. **Final report** — per-train summary: status, arrival time, total delay

---

## Class Diagram

![Class Diagram](diagrams/class_diagram.png)

Source: [diagrams/class_diagram.puml](diagrams/class_diagram.puml)

---

## Business Logic Diagrams

### Train Lifecycle — State Machine

Shows how a train transitions through states: Waiting → Running → Stopped/Delayed → Arrived.

![Train Lifecycle](diagrams/state_train_lifecycle.png)

Source: [diagrams/state_train_lifecycle.puml](diagrams/state_train_lifecycle.puml)

### Physics Engine — Single Tick Decision

The decision flowchart for each 1-second physics tick: brake, accelerate, or maintain.

![Physics Tick](diagrams/activity_physics_tick.png)

Source: [diagrams/activity_physics_tick.puml](diagrams/activity_physics_tick.puml)

### Simulation — High-Level Flow

End-to-end activity diagram: loading → initialisation → concurrent physics loop → finalisation.

![Simulation Overview](diagrams/activity_simulation_overview.png)

Source: [diagrams/activity_simulation_overview.puml](diagrams/activity_simulation_overview.puml)

---

## Sequence Diagrams

### Full Simulation Run

Shows the complete lifecycle: loading → path computation → train simulation → results.

![Simulation Run Sequence](diagrams/sequence_simulation_run.png)

Source: [diagrams/sequence_simulation_run.puml](diagrams/sequence_simulation_run.puml)

### Input Parsing

Details how the two input files are parsed into the domain model, including duration/time parsing and factory validation.

![Input Parsing Sequence](diagrams/sequence_input_parsing.png)

Source: [diagrams/sequence_input_parsing.puml](diagrams/sequence_input_parsing.puml)

### Dijkstra Pathfinding

The shortest-path algorithm: validation, priority queue loop, and path reconstruction.

![Pathfinding Sequence](diagrams/sequence_pathfinding.png)

Source: [diagrams/sequence_pathfinding.puml](diagrams/sequence_pathfinding.puml)

---

## Architecture Overview

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
```

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
└── Simulation/                           Orchestrator: physics engine, concurrent sim
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
└── CombinationTest.cpp                   315 tests (exhaustive parameter combinations)
```

## Design Patterns

### Strategy — Pathfinding

`IPathfinding` defines the interface; `DijkstraPathfinding` is the concrete strategy. The algorithm is injected into `Simulation` at construction, making it trivial to swap (e.g., A\*, BFS). A `PathWeightMode` parameter controls whether edges are weighted by distance (km) or time (km ÷ km/h):

```cpp
enum class PathWeightMode { Distance, Time };

class IPathfinding {
public:
    virtual ~IPathfinding() = default;
    virtual std::vector<std::shared_ptr<Node>> findPath(
        const std::string &start, const std::string &end,
        const RailNetwork &network,
        PathWeightMode mode = PathWeightMode::Distance) const = 0;
};
```

### Factory — Train Creation

`TrainFactory::createTrain()` validates all 9 parameters before constructing a `Train`. It auto-assigns an incrementing unique **numeric ID** (starting at 1) to each train. Invalid input (empty name, zero weight, negative friction, zero forces, negative departure time, negative stop duration, same departure/arrival) throws `std::invalid_argument`.

### Observer — Simulation Output

`ISimulationObserver` defines callbacks for simulation events. `FileOutputObserver` is the concrete observer that writes per-train `.result` files with timestep logs and rail graphs:

```cpp
class ISimulationObserver {
public:
    virtual ~ISimulationObserver() = default;
    virtual void onTrainStart(const std::string &trainName, int trainId, double estimatedTimeSec) = 0;
    virtual void onTrainStep(double timeSinceStart, const std::string &from,
                             const std::string &to, double distRemainingKm,
                             const std::string &action, int positionCellKm,
                             int segmentCellsKm, const std::vector<int> &blockingCells) = 0;
    virtual void onTrainEvent(const std::string &eventName, const std::string &nodeName,
                              double delaySec) = 0;
    virtual void onTrainFinish(const std::string &trainName, double totalTimeSec) = 0;
};
```

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

Tracks a train's journey with physics properties. Key fields:
- **id** (unique numeric identifier, auto-assigned by factory)
- **weight** (metric tons), **frictionCoefficient** (μ), **maxAccelForce** / **maxBrakeForce** (kN)
- **stopDuration** (seconds at each intermediate station)
- **currentSpeed** (m/s), **posOnSegment** (metres on current edge)
- **path** (sequence of `Node`s), **pathIndex**, **status**, accumulated **totalDelay**

Physics helpers:
- `getAccelRate()` → $(F_{accel} \times 1000 - \mu \cdot m \times 1000 \cdot 9.81) / (m \times 1000)$ m/s²
- `getDecelRate()` → $(F_{brake} \times 1000 + \mu \cdot m \times 1000 \cdot 9.81) / (m \times 1000)$ m/s²

### TrainState (struct)

Runtime simulation state per train, used by the `Simulation` loop:

| Field | Type | Description |
|---|---|---|
| `train` | `Train*` | Pointer to the train |
| `segmentIndex` | `size_t` | Current edge in path |
| `posOnSegment_m` | `double` | Metres from edge start |
| `speed_ms` | `double` | Current speed (m/s) |
| `timeSinceDepart` | `double` | Seconds since departure |
| `stopTimer` | `double` | Seconds remaining at station |
| `departed` | `bool` | Has the train left yet? |
| `arrived` | `bool` | Has it reached destination? |

### Event

Probabilistic disruption bound to a node or a rail segment. A **node event** has a single `_nodeName`; a **rail event** additionally stores `_nodeName2` (the other end of the segment). `isRailEvent()` returns `true` when `_nodeName2` is non-empty. `tryTrigger(rng)` rolls against `_probability` using a uniform distribution. Duration is in seconds.

### InputHandler

Static utility. Parses two text files into a `SimulationData` struct. Train format: `name weight friction accelForce brakeForce departure arrival time stopDuration`. Event format: `Event <name> <probability> <duration> <node> [node2]`. Supports quoted event names (`"Passenger's Discomfort"`), duration units (`30m`, `48h`, `356d`), and time format (`14h10`). An optional second node name makes the event a rail segment event.

### Simulation

Orchestrator with physics engine. Flow:
1. Compute shortest paths via injected `IPathfinding`
2. Create `FileOutputObserver` per train and `TrainState` per train
3. Run concurrent simulation loop (all trains, shared clock, DT=1s):
   - Check departures
   - Handle station stops (countdown timer)
   - Update physics (accelerate / brake / maintain)
   - Handle segment transitions (events, stops)
   - Output snapshots every 60 seconds via observers
   - Apply overtaking/blocking constraints
4. Finalize observer files and print console results

### FileOutputObserver

Concrete `ISimulationObserver` that writes per-train `.result` files. Filename: `TrainName_HHhMM.result`. Contains: header, timestep lines with rail graph, event entries, footer with actual travel time.

### OutputManager

Stateless formatter for console output. `formatTime(double seconds)` converts to `HH:MM:SS`. All output goes to `std::cout`.

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
