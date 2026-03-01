# Business Logic

> Back to [TECHNICAL.md](TECHNICAL.md) · [README](../README.md)

---

## What the Simulator Models

The system models a **rail network** where trains travel between stations along tracks using realistic physics, finding the fastest routes and encountering random disruptions along the way. All trains are simulated **concurrently** with a shared clock. It answers: *"Given a network, train specifications, and a schedule, when does each train arrive, and how do physics, stops, and disruptions affect travel time?"*

---

## Core Business Rules

### 1. Network Definition

A rail network consists of **stations** (nodes) connected by **tracks** (edges). Each track has:
- A **distance** in kilometres (must be positive)
- A **speed limit** in km/h (must be positive)

Connections are **bidirectional** — if CityA connects to CityB, CityB also connects to CityA. Self-loops (a station connected to itself) are forbidden. Duplicate connections between the same pair of stations are rejected.

### 2. Train Specification

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

### 3. Route Selection

Before simulation begins, each train is assigned an **optimal route** through the network using Dijkstra's algorithm. The cost metric is configurable via `PathWeightMode`:

- **Distance** (default) — minimises total kilometres: edge weight = `distance`
- **Time** — minimises travel time: edge weight = `distance / speedLimit`
- **Congestion** — dynamic re-routing that avoids occupied segments (see §3a)

Pass `--time` or `--congestion` on the command line (mutually exclusive). If no route exists, the train is skipped with a warning.

### 3a. Congestion-Aware Routing

When `--congestion` is active, the simulator dynamically re-routes trains at node transitions to avoid occupied segments:

1. **Base cost** = `distance / speedLimit` (identical to Time mode)
2. **Congestion penalty** = `120 seconds × (number of trains on segment) / 3600` hours, added per occupied segment
3. **Segment occupancy** is tracked as a snapshot each tick: `map<"from->to", count>`

**Performance guards** (to prevent pathological re-routing on dense networks):
- **Tick-level caching**: occupancy is built once per tick, not per-transition
- **Self-exclusion**: a train does not count itself when evaluating its own segments
- **Ahead-check**: re-routing only triggers if a remaining segment on the current path is actually congested
- **Cooldown**: minimum 3 segments must elapse between reroutes (prevents path oscillation)
- **Diff-check**: the new path is only applied if it differs from the current one

### 4. Physics-Based Travel Time

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

### 5. Station Stops

When a train reaches an intermediate node (not the final destination), it stops for the configured **stop duration**. During stops, the train remains at position 0 on the next segment with speed 0. Output lines during stops show action `Stopped`.

### 6. Concurrent Multi-Train Simulation

All trains are simulated **simultaneously** on a shared wall clock. Each train has its own `TrainState` tracking: segment index, position on segment (metres), speed (m/s), time since departure, stop timer, and departed/arrived flags.

### 7. Overtaking / Blocking

When two trains occupy the **same segment** in the same direction, the train behind has its speed capped to the speed of the train ahead. In the output rail graph, blocking trains appear as `[O]` cells.

### 8. Random Events (Disruptions)

Events can occur at **specific stations** (node events) or on **rail segments** between two stations (rail events). Each event has:
- A **probability** (0.0 to 1.0) of triggering
- A **duration** (the delay in seconds if triggered)

**Node events** trigger when a train arrives at the bound station. **Rail events** trigger when a train enters the bound segment (bidirectional match). Input format:

- Node event: `Event <name> <probability> <duration> <node_name>`
- Rail event: `Event <name> <probability> <duration> <node_name1> <node_name2>`

When triggered, the train's clock advances by the event's duration, and the delay is accumulated.

### 9. Per-Train Output Files (Observer Pattern)

Each train produces a result file named `TrainName_DepartureTime.result` containing:
- **Header**: train name + ID + estimated travel time
- **Timestep lines** (every 60 seconds): `[time] - [from][to] - [dist remaining] - [action] - [rail graph]`
- **Rail graph**: 1 cell per km — `[x]` = this train's position, `[O]` = blocking train, `[ ]` = empty
- **Event notifications** when triggered
- **Footer**: actual travel time

### 10. Train State Machine

| State | Meaning |
|---|---|
| **Waiting** | Created, not yet departed |
| **Running** | Departed, travelling along route |
| **Delayed** | An event added delay to this train |
| **Arrived** | Reached the arrival station |

### 11. Console Output

The simulator also produces console output:
1. **Network map** — all stations and their connections
2. **Route assignments** — the shortest path computed for each train
3. **Simulation log** — timestamped departures, arrivals, and events
4. **Final report** — per-train summary: status, arrival time, total delay

### 11a. Input Validation

- Both input files **must** have a `.txt` extension. The program exits with an error if this check fails.
- The `--help` flag may appear at **any argument position** and prints a full usage guide covering network file format, train file format, all CLI flags, and output structure.
- Unknown CLI options cause an immediate error.

---

## Bonus Business Rules

These rules extend the core simulation with additional capabilities.

### 12. Graph Export (`--graph`)

Generates a **Graphviz DOT** file visualising the rail network with computed paths:
- All stations rendered as nodes (city-prefixed nodes highlighted in blue)
- All tracks rendered as undirected edges with distance/speed labels
- Each train's Dijkstra-computed path overlaid in a unique colour from an 8-colour palette: **red, green, blue, orange, purple, cyan, magenta, lime** (with wraparound)
- Dark-themed graph matching the GUI aesthetic
- If the `dot` command is available, PNG and SVG are auto-rendered via `system()` call
- **Default directory:** if the filename contains no `/`, it is placed in `output/graphs/` automatically
- Export happens **once** on run 0 when combined with `--runs N`

### 13. Multi-Run Statistical Analysis (`--runs N`)

Extends the single-run simulation into a **Monte Carlo analysis**:
- Re-creates the full simulation pipeline N times, each with a **fresh random seed** (different `mt19937` state)
- Events trigger independently each run — same probabilities, different outcomes
- Run 0 produces normal output (files, console, animation); runs 1..N-1 are **silent** (only stats collected)
- Per-train statistics computed: **average**, **min**, **max** actual travel time + **average delay**
- Transforms the simulator from a single-point estimate into a probabilistic analysis tool

### 14. Terminal Animation (`--animate`)

Real-time visualisation of the simulation in the terminal:
- The `Simulation` engine accepts an **animation callback** via `setAnimCallback()` — called every physics tick with `(simTime, trainStates)`
- `TerminalAnimDisplay` renders each frame using **ANSI escape codes** (colours, cursor movement) and **Unicode box-drawing** characters
- **In-place rendering**: cursor moves to HOME position each frame — no scrolling, no flicker
- Frame rate: ~20 FPS (`usleep(50000)` per frame)
- Fully decoupled: `TerminalAnimDisplay` has zero dependency on `Simulation` — it only consumes a `vector<TrainState>` snapshot

### 15. GUI Interactive Simulation

The Qt GUI adds a complete interactive layer on top of the simulation engine:
- **Deferred validation**: network and train files can be loaded independently; cross-validation (station existence, route validity) only happens at simulation **Run** time
- **Background threading**: simulation runs in a `QThread` via the Worker Object pattern — the UI remains responsive at all times
- **Atomic speed control**: `std::atomic<double>` speed multiplier adjustable during simulation via logarithmic slider (0.1× to 10×)
- **Atomic stop**: `std::atomic<bool>` flag checked every frame; throws `SimulationStopException` for clean abort
- **Constant frame rate**: the GUI animation callback uses a fixed ~50 ms delay regardless of speed multiplier — speed is applied by advancing more simulated time per frame, not by changing the real-time delay
- **Live graph interpolation**: train positions are calculated by interpolating along graph edges based on segment progress fraction, giving smooth visual movement
