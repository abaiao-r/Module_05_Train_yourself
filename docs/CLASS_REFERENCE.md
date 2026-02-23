# Class Reference

> Back to [TECHNICAL.md](TECHNICAL.md) · [README](../README.md)

---

## Edge (struct)

Header-only POD in `src/Edge/Edge.hpp`:

| Member | Type | Description |
|---|---|---|
| `destination` | `shared_ptr<Node>` | Target node |
| `distance` | `double` | Kilometres |
| `speedLimit` | `double` | km/h |

---

## Node

A station in the rail network. Throws `InvalidNameException` on empty name.

| Method | Description |
|--------|-------------|
| `Node(name)` | Construct with name validation |
| `getName()` | Return station name |

---

## RailNetwork

Undirected graph stored as `unordered_map<string, shared_ptr<Node>>` + `unordered_map<string, vector<Edge>>`. Validates: no self-loops, positive distance/speed, no duplicates.

**Custom exceptions:** `NodeNotFoundException`, `DuplicateNodeException`, `DuplicateConnectionException`.

| Method | Description |
|--------|-------------|
| `addNode(name)` | Add a station |
| `addConnection(from, to, dist, speed)` | Add bidirectional track |
| `findNode(name)` | Lookup node by name |
| `getNeighbours(name)` | Get all edges from a node |
| `getNodeNames()` | List all station names |
| `nodeCount()` | Number of stations |

---

## Train

Tracks a train's journey with physics properties.

**Key fields:**
- **id** (unique numeric identifier, auto-assigned by factory)
- **weight** (metric tons), **frictionCoefficient** (μ), **maxAccelForce** / **maxBrakeForce** (kN)
- **stopDuration** (seconds at each intermediate station)
- **currentSpeed** (m/s), **posOnSegment** (metres on current edge)
- **path** (sequence of `Node`s), **pathIndex**, **status**, accumulated **totalDelay**

**Physics helpers:**
- `getAccelRate()` → $(F_{accel} \times 1000 - \mu \cdot m \times 1000 \cdot 9.81) / (m \times 1000)$ m/s²
- `getDecelRate()` → $(F_{brake} \times 1000 + \mu \cdot m \times 1000 \cdot 9.81) / (m \times 1000)$ m/s²

**Full public API:**

| Method | Description |
|--------|-------------|
| `Train(id, name, weight, friction, accel, brake, departure, arrival, time, stop)` | 10-parameter constructor |
| `Train(src)` / `operator=(src)` | Copy constructor / assignment |
| `~Train()` | Destructor |
| `setPath(path)` | Assign the computed route (vector of Nodes) |
| `advanceToNextNode(travelTime)` | Move to the next segment, increment path index |
| `applyDelay(seconds)` | Add delay seconds to total delay + current time |
| `setStatus(status)` | Change train state (Waiting/Running/Delayed/Arrived) |
| `setCurrentSpeed(speed)` | Update speed (m/s) |
| `setPosOnSegment(pos)` | Update position on current edge (metres) |
| `setPathIndex(idx)` | Set current segment index |
| `setCurrentTime(t)` | Set the train's clock |
| `getId()` | Unique numeric ID |
| `getName()` | Train name |
| `getWeight()` / `getFrictionCoefficient()` | Physics properties |
| `getMaxAccelForce()` / `getMaxBrakeForce()` | Force limits (kN) |
| `getDepartureStation()` / `getArrivalStation()` | Endpoints |
| `getDepartureTime()` / `getStopDuration()` | Schedule (seconds) |
| `getStatus()` | Current `TrainStatus` |
| `getPath()` / `getPathIndex()` | Route and progress |
| `getCurrentTime()` / `getTotalDelay()` | Clock and accumulated delay |
| `getCurrentSpeed()` / `getPosOnSegment()` | Physics state |
| `getCurrentNodeName()` | Name of the node at current path index |
| `hasArrived()` | True if status == Arrived |
| `getAccelRate()` / `getDecelRate()` | Computed acceleration/deceleration (m/s²) |
| `statusToString(status)` | Static: convert `TrainStatus` to string |

---

## TrainStatus (enum class)

Defined in `Train.hpp`. Represents the train's lifecycle state.

| Value | Meaning |
|-------|---------|
| `Waiting` | Created, not yet departed |
| `Running` | Departed, travelling along route |
| `Arrived` | Reached the arrival station |
| `Delayed` | An event added delay to this train |

---

## TrainState (struct)

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

---

## TrainResult (struct)

Defined in `Simulation.hpp`. Stores per-train summary after a simulation run.

| Field | Type | Description |
|---|---|---|
| `name` | `string` | Train name |
| `id` | `int` | Unique train ID |
| `estimatedTime` | `double` | Dijkstra-estimated travel time (seconds) |
| `actualTime` | `double` | Actual simulated travel time (seconds) |
| `totalDelay` | `double` | Accumulated event delay (seconds) |

---

## PathWeightMode (enum class)

Defined in `IPathfinding.hpp`. Controls Dijkstra edge weighting.

| Value | Edge weight |
|-------|-------------|
| `Distance` | `edge.distance` (km) — default |
| `Time` | `edge.distance / edge.speedLimit` (hours) |

---

## AnimTickCallback (type alias)

Defined in `Simulation.hpp`:

```cpp
using AnimTickCallback = std::function<void(double simTime,
                                            const std::vector<TrainState> &)>;
```

Callback invoked every physics tick when animation is enabled (`--animate` or GUI). Receives the current simulation time and a snapshot of all train states.

---

## Event

Probabilistic disruption bound to a node or a rail segment.

- A **node event** has a single `_nodeName`
- A **rail event** additionally stores `_nodeName2` (the other end of the segment)
- `isRailEvent()` returns `true` when `_nodeName2` is non-empty
- `tryTrigger(rng)` rolls against `_probability` using a uniform distribution
- Duration is in seconds

---

## TrainFactory

Static utility. Validates all 9 parameters and auto-assigns incrementing IDs.

| Method | Description |
|--------|-------------|
| `createTrain(...)` | Validate + construct a `unique_ptr<Train>` |
| `resetIdCounter()` | Reset ID to 1 (for testing) |

---

## InputHandler

Static utility. Parses two text files into a `SimulationData` struct.

- **Train format:** `name weight friction accelForce brakeForce departure arrival time stopDuration`
- **Event format:** `Event <name> <probability> <duration> <node> [node2]`
- Supports quoted event names (`"Passenger's Discomfort"`)
- Duration units: `30m`, `48h`, `356d`
- Time format: `14h10`
- An optional second node name makes the event a rail segment event

**Custom exception:** `ParseException` (extends `std::runtime_error`) — thrown on malformed input, missing fields, or bad keywords.

| Method | Description |
|--------|-------------|
| `loadData(netFile, trainFile)` | Parse both files → `SimulationData` |
| `parseNetworkFile(file, net, events)` | Parse network file only (public, for GUI) |
| `parseTrainFile(file)` | Parse train file only (public, for GUI) |

---

## SimulationData (struct)

Defined in `InputHandler.hpp`. Returned by `InputHandler::loadData()`.

| Field | Type | Description |
|---|---|---|
| `network` | `RailNetwork` | Parsed rail network |
| `trains` | `vector<unique_ptr<Train>>` | Parsed and validated trains |
| `events` | `vector<Event>` | Parsed events |

---

## Simulation

Orchestrator with physics engine.

**Flow:**
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

| Method | Description |
|--------|-------------|
| `run()` | Execute the full simulation |
| `getNetwork()` | Access the rail network (const ref) |
| `getTrains()` | Access the train list (const ref to `vector<unique_ptr<Train>>`) |
| `getResults()` | Retrieve per-train `TrainResult` summaries |
| `setAnimCallback(cb)` | Register animation callback — see [BONUS_ANIMATE.md](BONUS_ANIMATE.md) |
| `setQuiet(q)` | Suppress console output — see [BONUS_MULTIRUN.md](BONUS_MULTIRUN.md) |

---

## FileOutputObserver

Concrete `ISimulationObserver` that writes per-train `.result` files. Non-copyable (owns `std::ofstream`).

- **Filename:** `TrainName_HHhMM.result`
- **Contents:** header, timestep lines with rail graph, event entries, footer with actual travel time

| Method | Description |
|--------|-------------|
| `FileOutputObserver(trainName, departureTime)` | Open result file for writing |
| `onTrainStart(...)` | Write header with estimated time |
| `onTrainStep(...)` | Write one timestep line with rail graph |
| `onTrainEvent(...)` | Write event trigger notification |
| `onTrainFinish(...)` | Write footer with actual travel time |
| `getFilename()` | Return the generated filename |

---

## OutputManager

Stateless formatter for console output.

| Method | Description |
|--------|-------------|
| `printNetwork(net)` | Print all stations and connections |
| `printTrainPath(train)` | Print computed route |
| `printDeparture(train)` | Print departure notification |
| `printArrival(train, node, time)` | Print station arrival |
| `printEvent(event, train)` | Print event trigger |
| `printResult(trains)` | Print final summary table |
| `formatTime(seconds)` | Convert seconds → `HH:MM:SS` |

---

## GraphExporter

Static utility for Graphviz DOT export.

| Method | Description |
|--------|-------------|
| `exportDot(filename, network, trains)` | Write DOT + auto-render PNG/SVG |

---

## TerminalAnimDisplay

Terminal animation renderer (bonus). Zero dependency on `Simulation`.

| Method | Description |
|--------|-------------|
| `render(simTime, states)` | Draw one frame |
| `cleanup()` | Restore cursor visibility |

---

## GUI Classes (Bonus)

See [BONUS_GUI.md](BONUS_GUI.md) for full details.

| Class | Description |
|-------|-------------|
| `MainWindow` | Main window: menus, toolbar, CRUD, simulation control |
| `NetworkScene` | Interactive graph visualisation (drag, layout, train rendering) |
| `SimulationWorker` | Background-threaded simulation runner (signals/slots) |
| `SimDashboard` | Live HTML dashboard widget showing train progress |
| `DraggableNode` | `QGraphicsEllipseItem` subclass for drag-and-drop nodes |

### TrainSnapshot (struct)

Defined in `SimulationWorker.hpp`. Emitted every tick from the worker thread to the GUI.

| Field | Type | Description |
|---|---|---|
| `name` | `QString` | Train name |
| `id` | `int` | Unique train ID |
| `from` / `to` | `QString` | Current segment endpoints |
| `departure` / `arrival` | `QString` | Journey endpoints |
| `posOnSegment_m` | `double` | Metres from segment start |
| `speed_ms` | `double` | Current speed (m/s) |
| `timeSinceDepart` | `double` | Seconds since departure |
| `stopTimer` | `double` | Seconds remaining at station |
| `departed` / `arrived` | `bool` | Journey flags |
| `segmentIndex` | `size_t` | Current edge in path |
| `pathSize` | `size_t` | Total edges in path |
| `NetworkScene` | Interactive `QGraphicsScene` with draggable nodes |
| `SimulationWorker` | Background-threaded simulation runner |
| `SimDashboard` | Live HTML dashboard widget |
