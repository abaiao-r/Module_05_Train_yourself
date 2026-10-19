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
- **Solo** — fastest route if this train were the only one on the track: edge weight = `distance / speedLimit`, ignores other trains and random events
- **Adaptive** — the ultimate mode: builds on Solo's time weighting, plus dynamically re-routes around occupied segments and avoids routes prone to costly random events (see §3a)

Pass `--distance`, `--solo`, or `--adaptive` on the command line (mutually exclusive; `--solo` is the default with no flag). If no route exists, the train is skipped with a warning.

### 3a. Adaptive Mode: Congestion + Event-Aware Routing

When `--adaptive` is active, the simulator **dynamically re-routes trains at runtime** to spread traffic across parallel corridors, avoid crowded segments, and steer away from routes prone to costly random events. This contrasts with Distance and Solo modes, where every train's path is fixed before the simulation starts and never changes.

#### The rule: "am I stuck, and does a detour pay?"

A train asks four questions, in order. The first "no" means **stay put** (no pathfinding, no cost):

1. Is another train on **my current segment**?
2. Is it **ahead of me** (it entered the segment before me; ties broken by train id)?
3. Am I **faster** than it (my free ETA to the segment end is shorter)?
4. Does the leader keep blocking me **after the next node** (shared stretch > 0), and is the best detour from that node at least `MIN_GAIN_SEC` (30 s) quicker than staying?

A train cannot turn around mid-segment, so the earliest switch point is the **next node** (the pivot). If the leader leaves the shared route at the pivot, nothing is gained and no search is done.

#### How the two options are compared

- **Stay ETA** and **detour ETA** both come from `replayEta`, a non-mutating replay of the real physics (acceleration, braking, node speed carry-over, dwell at intermediate stops, following distance behind the leader).
- The detour candidate is found with `findConstrainedPath`: Dijkstra from the pivot with the shared directed segments **forbidden during the search**, using static weights (`distance / speedLimit`, plus expected event delay, plus the train's stop duration at intermediate nodes).
- Dijkstra only **proposes** a candidate; `replayEta` **judges** it. Reroute iff `detourETA + 30 s < stayETA`.
- Random events are not replayed; their expected delay is added per remaining segment to both options.
- **Event risk = probability × duration** (the average delay per pass, in the same units as travel time). Example: a 40 % × 20 min event costs 8 min on average, so a route that is 5 min longer but avoids it wins; at 5 % the cost is 1 min and the shorter route wins. Break-even here is 25 %. This minimises the *average* delay of the train itself; it ignores variance and knock-on delay to following trains. The map is cached and rebuilt only when events or topology change live.

#### When it is evaluated (cost control)

The cheap checks (1-3 and shared-stretch) are pure arithmetic every tick. The expensive part (constrained Dijkstra + two replays) runs only on triggers, at most 3 times per train per segment: on **segment entry**, on **leader change / leader reroute / leader starts dwelling / topology or event change**, and once when the train is within `PRE_PIVOT_HORIZON_SEC` (60 s) of the pivot. Candidate paths are cached per (pivot, destination, forbidden set, dwell) and cleared when topology or events change; the decision itself is never cached. A 3-segment cooldown remains as secondary anti-churn.

A dwelling train does not block. Directional tracks: `A->B` and `B->A` are independent (head-on conflicts are out of scope).

#### Per-train ETA profile

Each train keeps an `etaProfile` (free-flow predicted arrival at each remaining node), rebuilt on route change, segment entry, dwell end and (rate-limited to once per 10 s) when blocking deviates the train. It is for display (GUI) only, not a decision input.

#### Demo scenarios

`input/*/railNetworkScenarioN*.txt` + `trainPrintScenarioN*.txt`: 1 Baseline, 2 BlockReroutePays, 3 BlockStayBetter, 4 MultiTrainCongestion, 5 EventRisk, 6 Combined, 7 LeaderDiverges, 8 SkipDivergingLeader (the nearest leader that keeps sharing the route after the pivot is used, not just the train directly ahead). Run e.g. `./bin/train_yourself <network> <train> --adaptive` or load them in the GUI. Tests: `tests/AdaptiveRerouteTest.cpp`.

#### Design rationale

**Why 120 seconds (`CONGESTION_PENALTY`)?**
The penalty must be large enough to push trains onto an alternative route, but small enough that a mildly congested short path can still beat a much longer empty path. On a typical segment (say 50 km at 200 km/h → 15 min travel time), one extra train adds 120 s ≈ 2 min of virtual cost — roughly a 13 % surcharge. Two trains add 4 min (~27 %). This is in the same order of magnitude as **real-world operational delays caused by headway constraints** on shared track (trains must keep 2–3 min separation in practice, and each following train incurs signalling-imposed slowdowns). The constant is deliberately conservative: it biases towards keeping the fast route unless congestion is significant. A value too low (e.g., 30 s) would make rerouting almost never trigger; a value too high (e.g., 600 s) would scatter trains onto poor routes at the slightest sharing.

**Why decentralised (no central coordinator)?**
A central scheduler (e.g., a conflict-free timetable planner or slot-allocation system) would be more "optimal" in a global sense, but it contradicts the simulator's core design: trains are independent agents that depart at scheduled times and react to local conditions. The decentralised approach mirrors **real-world dispatcher-less signalling** where each train's on-board system picks routes based on current track occupancy (similar to ETCS Level 3 concepts). It is also simpler to implement, test, and reason about — each train independently asks "is my remaining path congested?" and, if so, runs Dijkstra with penalties. No global lock, no train-to-train communication, no second pass. The emergent behaviour (load balancing) arises from each train independently penalising the same congested segments.

**Why a cooldown of 3 segments?**
Without a cooldown, two trains departing 1 minute apart can enter an infinite reroute loop:
1. Train A takes Route 1, Train B sees congestion on Route 1 → switches to Route 2.
2. Next segment, Train A sees Route 2 is now congested (Train B is there) → switches to Route 2 as well.
3. Train B sees Route 2 has 2 trains → switches back to Route 1. Repeat forever.

A cooldown of 3 segments means a train must commit to its chosen corridor for at least 3 hops before reconsidering. On most networks, 3 segments is enough to clear the overlap zone of a fork, so by the time the cooldown expires the train has "passed through" the shared area and won't flip back. The value 3 was chosen empirically: 1 is too short (oscillation still occurs on tight meshes), 5+ is too long (trains miss valid reroute opportunities on networks with many short segments). Three provides a good balance for the test networks ranging from 3-node minimal to 20-node ParisMetro.

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

**Node events** trigger when a train arrives at the bound station. **Rail events** trigger when a train enters the bound segment (bidirectional match), including its first segment at departure. Input format:

- Node event: `Event <name> <probability> <duration> <node_name>`
- Rail event: `Event <name> <probability> <duration> <node_name1> <node_name2>`

When triggered, the train's clock advances by the event's duration, and the delay is accumulated.

Note: a train that finishes a station stop is re-queued behind any train that overtook it during the stop, so it never drags a faster train backwards.

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
