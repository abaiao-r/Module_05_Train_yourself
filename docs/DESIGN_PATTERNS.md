# Design Patterns

> Back to [TECHNICAL.md](TECHNICAL.md) · [README](../README.md)

---

## Strategy — Pathfinding

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

**Why Strategy?** — Decouples route computation from the simulation engine. Adding a new algorithm (A\*, BFS, Bellman-Ford) requires only a new class implementing `IPathfinding`.

---

## Factory — Train Creation

`TrainFactory::createTrain()` validates all 9 parameters before constructing a `Train`. It auto-assigns an incrementing unique **numeric ID** (starting at 1) to each train. Invalid input (empty name, zero weight, negative friction, zero forces, negative departure time, negative stop duration, same departure/arrival) throws `std::invalid_argument`.

```cpp
class TrainFactory {
    static int _nextId;
public:
    static std::unique_ptr<Train> createTrain(
        const std::string &name, double weight, double friction,
        double accelForce, double brakeForce,
        const std::string &departure, const std::string &arrival,
        double departureTime, double stopDuration);
    static void resetIdCounter();
};
```

**Why Factory?** — Centralises all validation logic in one place. Guarantees every `Train` in the system was validated at creation time. The auto-incrementing ID ensures uniqueness without manual tracking.

---

## Observer — Simulation Output

`ISimulationObserver` defines callbacks for simulation events. `FileOutputObserver` is the concrete observer that writes per-train `.result` files with timestep logs and rail graphs:

```cpp
class ISimulationObserver {
public:
    virtual ~ISimulationObserver() = default;
    virtual void onTrainStart(const std::string &trainName, int trainId,
                              double estimatedTimeSec) = 0;
    virtual void onTrainStep(double timeSinceStart, const std::string &from,
                             const std::string &to, double distRemainingKm,
                             const std::string &action, int positionCellKm,
                             int segmentCellsKm,
                             const std::vector<int> &blockingCells) = 0;
    virtual void onTrainEvent(const std::string &eventName,
                              const std::string &nodeName,
                              double delaySec) = 0;
    virtual void onTrainFinish(const std::string &trainName,
                               double totalTimeSec) = 0;
};
```

**Why Observer?** — Decouples the simulation engine from I/O. Adding a new output format (JSON, CSV, database) requires only a new observer implementation — zero changes to `Simulation`.

---

## Dependency Injection

`Simulation` receives all its dependencies through the constructor — `RailNetwork`, `Train`s, `Event`s, and `IPathfinding` — enabling unit-testable design without globals or singletons.

**Constructor signature:**
```cpp
Simulation(RailNetwork network,
           std::vector<std::unique_ptr<Train>> trains,
           std::vector<Event> events,
           std::unique_ptr<IPathfinding> pathfinder,
           PathWeightMode weightMode = PathWeightMode::Distance);
```

---

## Orthodox Canonical Form

All value-type classes implement the **canonical four** (Rule of Three + destructor). Move-only classes explicitly delete copy operations.

| Class | Param ctor | Copy ctor | Copy assign | Destructor | Move |
|-------|:---:|:---:|:---:|:---:|:---:|
| `Node` | ✓ | ✓ | ✓ | ✓ | — |
| `Train` | ✓ | ✓ | ✓ | ✓ | — |
| `Event` | ✓ | ✓ | ✓ | ✓ | — |
| `RailNetwork` | ✓ (default) | ✓ | ✓ | ✓ | ✓ (move ctor + assign) |
| `OutputManager` | ✓ (default) | ✓ | ✓ | ✓ | — |
| `DijkstraPathfinding` | ✓ (default) | ✓ | ✓ | ✓ | — |
| `Simulation` | ✓ | ✗ (deleted) | ✗ (deleted) | ✓ | — (move-only by design) |
| `FileOutputObserver` | ✓ | ✗ (deleted) | ✗ (deleted) | ✓ | — (owns `ofstream`) |
| `TerminalAnimDisplay` | ✓ (default) | ✗ (deleted) | ✗ (deleted) | ✓ | — (manages terminal state) |

> **Note:** `Node` and `Train` have **parameterised constructors only** (no zero-argument default). `Simulation`, `FileOutputObserver`, and `TerminalAnimDisplay` explicitly `= delete` copy operations because they manage unique resources (file handles, terminal state, `unique_ptr` members).

---

## Pattern Map

```
                  IPathfinding (Strategy)
                       │
                DijkstraPathfinding
                       │
    InputHandler ──► Simulation ──► ISimulationObserver (Observer)
         │               │                    │
    TrainFactory      RailNetwork      FileOutputObserver
    (Factory)         Train[]
                      Event[]
```

---

## Class Relationships

The class diagram uses standard UML relationship notation. Each arrow type maps to a specific C++ ownership semantic. See the rendered [Class Diagram](DIAGRAMS.md#class-diagram) for the visual representation.

| UML Arrow | Relationship | Meaning | Lifetime |
|-----------|-------------|---------|----------|
| `*──` (filled diamond) | **Composition** | Parent **owns** the child; child cannot exist without parent | Same as parent |
| `──>` (solid arrow) | **Association** | Class **uses / references** another; no ownership | Independent |
| `..>` (dashed arrow) | **Dependency** | Class **temporarily uses** another (parameter, local) | Transient |
| `..\|>` (dashed triangle) | **Realisation** | Class **implements** an interface | — |
| `──\|>` (solid triangle) | **Inheritance** | Class **extends** a base class | — |

### Concrete examples in this project

| Arrow | From → To | C++ Mechanism |
|-------|-----------|---------------|
| Composition `*──` | `RailNetwork *── Node` | `unordered_map<string, shared_ptr<Node>>` — network owns its nodes |
| Composition `*──` | `RailNetwork *── Edge` | `unordered_map<string, vector<Edge>>` — network owns its edges |
| Composition `*──` | `Simulation *── Train` | `vector<unique_ptr<Train>>` — simulation owns its trains |
| Composition `*──` | `Simulation *── Event` | `vector<Event>` — simulation owns its events |
| Composition `*──` | `MainWindow *── NetworkScene` | Qt parent-child ownership |
| Composition `*──` | `MainWindow *── SimDashboard` | Qt parent-child ownership |
| Association `──>` | `Edge ──> Node` | `shared_ptr<Node> destination` — edge references a node |
| Association `──>` | `Train ──> Node` | `vector<shared_ptr<Node>> _path` — train references nodes |
| Association `──>` | `Simulation ──> IPathfinding` | `unique_ptr<IPathfinding>` — injected strategy |
| Association `──>` | `MainWindow ──> SimulationWorker` | controls via signals/slots |
| Dependency `..>` | `SimulationWorker ..> Simulation` | creates internally per run |
| Dependency `..>` | `InputHandler ..> TrainFactory` | calls static factory method |
| Dependency `..>` | `NetworkScene ..> TrainSnapshot` | renders snapshot data |
| Realisation `..\|>` | `DijkstraPathfinding ..\|> IPathfinding` | implements pure virtual interface |
| Realisation `..\|>` | `FileOutputObserver ..\|> ISimulationObserver` | implements observer interface |
| Inheritance `──\|>` | `Node::InvalidNameException ──\|> std::invalid_argument` | extends standard exception |

---

## Design Principles

### KISS — Keep It Simple, Stupid

Every component does **one thing** with a straightforward implementation:

| Where | How KISS is applied |
|-------|-------------------|
| **Edge** | A plain `struct` with 3 public fields — no getters, no methods, no inheritance. |
| **Event** | Just a name, a node, and a duration — stored as a flat value type. |
| **TrainFactory** | One static method `createTrain()` — validates, constructs, returns. No builder pattern, no configuration objects. |
| **PathWeightMode** | A simple `enum class` with two values instead of a class hierarchy for weight strategies. |
| **Physics loop** | A single `while` loop with clear `if/else` branches (brake → accelerate → maintain). No state machine framework. |

### DRY — Don't Repeat Yourself

Shared logic is extracted into single authoritative locations:

| What | How DRY is applied |
|------|-------------------|
| **Train creation** | `TrainFactory` is the single validation point — neither `InputHandler` nor the GUI duplicate checks. |
| **Duration parsing** | One `parseDuration()` utility converts `"1h30m"`, `"90m"`, `"5400s"` — used by both `InputHandler` and event parsing. |
| **Result file writing** | `FileOutputObserver` is the single writer — `Simulation` never formats output strings. |
| **Path computation** | `IPathfinding::findPath()` is called once per train; the result is stored, never recomputed. |
| **ID generation** | `TrainFactory::_nextId` is the one source of truth for unique train IDs — no duplicate counters elsewhere. |
| **Orthodox Canonical Form** | Copy constructors/assignment operators are implemented in a consistent pattern across all value types. |

### Encapsulation

All class members are **private** with controlled access:

```cpp
class Train {
private:
    int         _id;                  // set once by TrainFactory
    std::string _name;                // immutable after construction
    double      _weight;              // validated at creation
    double      _frictionCoefficient;
    double      _maxAccelForce;       // kilonewtons
    double      _maxBrakeForce;       // kilonewtons
    TrainStatus _status;              // only modifiable via setStatus()
    // ...
public:
    int         getId() const;
    std::string getName() const;
    TrainStatus getStatus() const;
    void        setStatus(TrainStatus status);   // controlled state transition
    void        advanceToNextNode(double travelTime);
    void        applyDelay(double seconds);
};
```

**Key encapsulation boundaries:**

| Class | What is hidden | Public contract |
|-------|---------------|----------------|
| `RailNetwork` | Internal graph representation (`unordered_map`) | `addNode()`, `addConnection()`, `getNeighbours()` |
| `Train` | Physics state (speed, position, forces) | `getId()`, `getName()`, `getStatus()`, state transition methods |
| `Node` | Internal name storage | `getName()` only — no setter, enforced by const reference |
| `Simulation` | Tick loop internals, train scheduling | `run()` — single entry point |
| `OutputManager` | File I/O handles, path construction | `writeResult()`, `setOutputDir()` |
| `DijkstraPathfinding` | Priority queue, visited set, distance map | `findPath()` — pure function, no side effects |

### Single Responsibility Principle (SRP)

Each class has **exactly one reason to change**:

| Class | Sole responsibility |
|-------|-------------------|
| `Node` | Represent a named station |
| `Edge` | Hold connection data (destination, distance, speed) |
| `Train` | Model a train's physical state and journey |
| `Event` | Store delay event data |
| `RailNetwork` | Maintain the graph topology |
| `InputHandler` | Parse input files into domain objects |
| `TrainFactory` | Validate and construct trains |
| `DijkstraPathfinding` | Compute shortest paths |
| `FileOutputObserver` | Write simulation results to files |
| `OutputManager` | Manage output directory and file routing |
| `Simulation` | Run the physics loop |
| `GraphExporter` | Export the network as a DOT graph |

### Separation of Concerns

The architecture enforces strict layering so that concerns never leak across boundaries:

```
┌─────────────────────────────────────────┐
│  Presentation  (FileOutputObserver,     │
│                 Terminal Animation,     │
│                 Qt GUI)                 │
├─────────────────────────────────────────┤
│  Orchestration (Simulation,             │
│                 InputHandler)           │
├─────────────────────────────────────────┤
│  Domain        (Node, Edge, Train,      │
│                 Event, RailNetwork)     │
├─────────────────────────────────────────┤
│  Strategy      (IPathfinding,           │
│                 DijkstraPathfinding)    │
└─────────────────────────────────────────┘
```

- **Domain** classes know nothing about I/O, files, or the GUI.
- **Strategy** classes know nothing about trains or events — only graph topology.
- **Presentation** classes depend on domain types but never modify them directly.

### Interface Segregation

Interfaces are **minimal and focused**:

- `IPathfinding` — one method: `findPath()`. No graph building, no caching API.
- `ISimulationObserver` — four callbacks: start, step, event, finish. No subscription management, no filtering.

Clients are never forced to depend on methods they don't use.
