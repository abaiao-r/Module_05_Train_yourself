# The Diagrams, Explained — with the Code

Twin of [DIAGRAMS_EXPLAINED.md](DIAGRAMS_EXPLAINED.md). Same sections, same order; here every idea is tied to the code that does it.
Line numbers are at the time of writing; search by function name if they drift. Snippets are condensed/paraphrased from the real code for readability (identifiers are real; check the referenced function for exact text).

---

## 1. The Cast (class diagram) — *where each class lives*

| Class | File |
|---|---|
| `RailNetwork`, `Node`, `Edge` | `src/RailNetwork/`, `src/Node/`, `src/Edge/` |
| `Train`, `TrainStatus` | `src/Train/Train.hpp` (enum at line 22) |
| `Event` | `src/Event/Event.cpp` |
| `InputHandler` | `src/InputHandler/InputHandler.cpp` |
| `TrainFactory` | `src/TrainFactory/TrainFactory.cpp` |
| `DijkstraPathfinding` | `src/DijkstraPathfinding/DijkstraPathfinding.cpp` |
| `Simulation`, `TrainState`, `TrainResult`, `LiveMutation` | `src/Simulation/Simulation.hpp/.cpp` |
| `FileOutputObserver` / `OutputManager` | `src/FileOutputObserver/`, `src/OutputManager/` |
| GUI (`SimulationWorker`, …) | `bonus/gui/` |

The per-train "notepad" the referee uses (`TrainState`, `Simulation.hpp:78`) holds the live facts:

```cpp
struct TrainState {
    Train *train;
    size_t segmentIndex;     // which track of its route it is on
    double posOnSegment_m;   // metres from the start of that track
    double speed_ms;
    double stopTimer;        // station dwell OR event hold countdown
    bool   heldBySegmentEvent;
    bool   departed, arrived;
    Train *blockedByTrain;   // who is capping my speed this tick
    int    rerouteCount;  int segsSinceReroute;  /* … */
};
```

The final score card (`Simulation.hpp`):

```cpp
struct TrainResult { std::string name; int id;
                     double estimatedTime, actualTime, totalDelay; };
```

---

## 2. A Train's Life (state diagram)

**The four real states** — `Train.hpp:22`:

```cpp
enum class TrainStatus { Waiting, Running, Arrived, Delayed };
```

**Waiting → Running** when the clock reaches the departure time (`Simulation::run`, ~line 313):

```cpp
if (!s.departed && simTime >= s.train->getDepartureTime()) {
    s.departed = true;
    s.train->setStatus(TrainStatus::Running);
    rollSegmentEvents(s, i);   // events on the first track
}
```

**"Stopped at a station" is a countdown, not a state** (`Simulation::run`, ~line 331):

```cpp
if (s.stopTimer > 0.0) {
    s.stopTimer -= DT;               // DT = 1 second
    if (s.stopTimer <= 0.0) { s.stopTimer = 0.0; /* back in the queue */ }
    s.timeSinceDepart += DT;
    isStopped = true;                // physics skipped this second
}
```

The countdown is loaded when a train reaches an intermediate station (`handleSegmentTransition`, ~line 745):

```cpp
s.stopTimer += s.train->getStopDuration();
if (s.stopTimer > 0.0) s.speed_ms = 0.0;
```

**Delayed = "capped by the train ahead", reset every tick** (`applyBlocking`, ~line 1213):

```cpp
if (st.train->getStatus() == TrainStatus::Delayed)
    st.train->setStatus(TrainStatus::Running);   // clear first…
/* … */
if (capped) {                                    // …set again if still blocked
    behind->train->setStatus(TrainStatus::Delayed);
    behind->blockedByTrain = ahead->train;
}
```

**Arrived** (`handleSegmentTransition`, ~line 735):

```cpp
if (isLast) { s.speed_ms = 0.0; s.arrived = true;
              s.train->setStatus(TrainStatus::Arrived); }
```

**GUI Stop** is an exception thrown from the callback, not a state (`bonus/gui/SimulationWorker.cpp:277`):

```cpp
if (_stopRequested.load(std::memory_order_relaxed))
    throw SimulationStopException();
```

---

## 3. One Second of Physics (activity diagram)

All in `Simulation::physicsStep` (`Simulation.cpp:646`).

**Steps 1–2. Target speed at the end of this track, and braking distance:**

```cpp
double targetEnd = 0.0;                       // last track: stop
bool isLast = (segIdx == path.size() - 2);
if (!isLast) {                                // else: next track's limit
    getSegmentInfo(path[segIdx+1]->getName(), path[segIdx+2]->getName(), nLen, nSpd);
    targetEnd = std::min(nSpd, speedLim_ms);
}
double brakeDist = 0.0;
if (speed > targetEnd)
    brakeDist = (speed*speed - targetEnd*targetEnd) / (2.0 * decel);
```

**Steps 3–5. Brake, accelerate, or cruise:**

```cpp
if (distToEnd <= brakeDist + 2.0 && speed > targetEnd + 0.1) {
    newSpeed = std::max(speed - decel * DT, targetEnd);          // brake
} else if (speed < speedLim_ms - 0.1) {
    newSpeed = std::min(speed + accel * DT, speedLim_ms);        // accelerate
} else newSpeed = speedLim_ms;                                    // cruise
```

**Step 6. Move (average speed over the second):**

```cpp
pos += (speed + newSpeed) / 2.0 * DT;
```

Where `accel` and `decel` come from the train's physics (`src/Train/Train.cpp:155`): engine force minus friction, divided by mass.

```cpp
double Train::getAccelRate() const {
    double mass_kg = _weight * 1000.0;
    double friction_N = _frictionCoefficient * mass_kg * 9.81;
    double net = _maxAccelForce * 1000.0 - friction_N;
    return (net > 0.0) ? net / mass_kg : 0.0;
}
double Train::getDecelRate() const {          // friction helps the brakes
    return (_maxBrakeForce*1000.0 + friction_N) / mass_kg;
}
```

**Step 7. Reached the end of the track?** (`run`, ~line 362 → `handleSegmentTransition`, `Simulation.cpp:699`). Events at the arrival station are rolled here:

```cpp
for (const auto *event : getEventsAtNode(arrNode)) {
    if (event->tryTrigger(_rng)) {            // the dice roll
        double delay = event->getDuration();
        if (isLast) { s.train->applyDelay(delay); s.timeSinceDepart += delay; }
        else        holdTrain(s, delay);      // stand still for `delay`
    }
}
```

The dice (`src/Event/Event.cpp:56`):

```cpp
bool Event::tryTrigger(std::mt19937 &rng) const {
    if (_probability <= 0.0) return false;
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(rng) < _probability;
}
```

**Step 8. A diary line once a minute** (`run`, ~line 375; `OUTPUT_INTERVAL = 60`):

```cpp
double modt = std::fmod(s.timeSinceDepart, OUTPUT_INTERVAL);
if (modt < DT || s.arrived) { /* … */ _observers[i]->onTrainStep(/* time, from, to, distRem, action … */); }
```

**Step 9. Tell the animation / GUI** (end of the tick):

```cpp
if (_animCallback) _animCallback(simTime, states);
```

In the GUI the callback is throttled: it emits a frame every 30 simulated seconds (scaled by the speed slider) and sleeps 50 ms after each frame (`SimulationWorker.cpp:270`):

```cpp
static constexpr double BASE_INTERVAL = 30.0;      // sim seconds per frame
double interval = BASE_INTERVAL * spd;
if (simTime - lastEmit < interval) return;
```

---

## 4. The Whole Run, Start to Finish (activity diagram)

`Simulation::run` (`Simulation.cpp:235`) is the diagram in code form:

```cpp
void Simulation::run() {
    computePaths();                                   // 2. routes
    std::sort(_trains.begin(), _trains.end(), /* by departure time */);
    /* 3. one TrainState + FileOutputObserver per train */
    for (...) _observers[i]->onTrainStart(name, id, estimateTravelTime(*train));
    double simTime = earliestDeparture - DT;

    while (anyActive()) {                             // 4. the clock
        simTime += DT;
        for (auto &m : drainMutations()) applyMutation(m, states, simTime);  // live edits
        /* depart / dwell / physics / events / diary line, per train */
        SegmentState segState = buildSegmentState(states);
        if (_weightMode == PathWeightMode::Adaptive)
            for (...) considerReroute(states, idx, group, k, simTime);
        applyBlocking(states, segState, simTime);     // nobody drives through anybody
        if (_animCallback) _animCallback(simTime, states);
    }
    for (...) _observers[i]->onTrainFinish(name, timeSinceDepart);   // 6. close diaries
    if (!_quiet) _output.printResult(_trains);
    /* fill _results with TrainResult */
}
```

**The "best-case" estimate** (`estimateTravelTime`, `Simulation.cpp:557`): per track, accelerate to the limit, cruise, brake. If the track is too short to reach the limit, use the peak speed that fits:

```cpp
if (dAccel + dBrake > segLen_m) {
    double vMax = std::sqrt(2.0 * segLen_m * accel * decel / (accel + decel));
    totalTime += vMax / accel + vMax / decel;
} else totalTime += tAccel + (segLen_m - dAccel - dBrake)/speedLim_ms + tBrake;
if (i < path.size() - 2) totalTime += train.getStopDuration();
```

**Graph picture after the run, first run only** (`src/main.cpp:316`):

```cpp
sim.run();
if (run == 0 && !graphFile.empty())
    GraphExporter::exportDot(graphFile, sim.getNetwork(), sim.getTrains());
```

**Multi-run statistics** (`main.cpp:300`): same-named trains get `Name`, `Name #2`, … rows:

```cpp
int n = seen[r.name]++;
std::string key = n ? r.name + " #" + std::to_string(n + 1) : r.name;
statActual[key].push_back(r.actualTime);
```

---

## 5. Choosing a Route (sequence diagram)

**Who asks** — `Simulation::computePaths` (`Simulation.cpp:509`):

```cpp
SegmentEventRisk eventRisk;
if (_weightMode == PathWeightMode::Adaptive) eventRisk = buildEventRisk();
for (auto &train : _trains) {
    path = (_weightMode == PathWeightMode::Adaptive)
         ? _pathfinder->findPath(dep, arr, _network, _weightMode, SegmentOccupancy{}, {}, eventRisk)
         : _pathfinder->findPath(dep, arr, _network, _weightMode);
    if (path.empty()) throw std::runtime_error("No path found for " + train->getName() + …);
    train->setPath(path);
}
```

**What "cheap" means** — `DijkstraPathfinding::edgeWeight` (`DijkstraPathfinding.cpp:35`):

```cpp
if (mode == Solo || mode == Adaptive) return edge.distance / edge.speedLimit; // hours
return edge.distance;                                                         // km
```

and for adaptive mode (line 43) the penalties on top:

```cpp
if (occupied)  base += max(estimatedClearTime, MIN_CONGESTION_PENALTY) / 3600.0;
if (eventRisk) base += expectedDelaySeconds / 3600.0;   // probability × duration
```

**The "water spreading out" algorithm** — `findPath` (`DijkstraPathfinding.cpp:78`):

```cpp
std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> pq;   // cheapest first
dist[start] = 0.0;  pq.push({0.0, start});
while (!pq.empty()) {
    auto [d, u] = pq.top(); pq.pop();
    if (d > dist[u]) continue;            // stale entry
    if (u == end) break;                  // cheapest route found
    for (const auto &edge : network.getNeighbours(u)) {
        double newDist = dist[u] + edgeWeight(edge, mode);
        if (newDist < dist[v]) { dist[v] = newDist; prev[v] = u; pq.push({newDist, v}); }
    }
}
```

**Expected event cost per track** (`buildEventRisk`, `Simulation.cpp`):

```cpp
for (const auto *ev : getEventsOnSegment(fromName, toName))
    expectedDelay += ev->getProbability() * ev->getDuration();
for (const auto *ev : getEventsAtNode(toName)) expectedDelay += /* same */;
```

---

## 6. Reading the Input Files (sequence diagram)

**Entry point** — `InputHandler::loadData` (`InputHandler.cpp:120`):

```cpp
SimulationData data;
parseNetworkFile(networkFile, data.network, data.events);   // cities, tracks, events
data.trains = parseTrainFile(trainFile);                    // trains
validateData(networkFile, trainFile, data);                 // cross-check
return data;
```

Bad input is rejected with the file and line (`throw ParseException(...)`, e.g. `InputHandler.cpp:136`).

**The train checklist** — `TrainFactory::createTrain` (`TrainFactory.cpp:22`):

```cpp
if (name.empty())                         throw std::invalid_argument("Train name cannot be empty");
if (name contains / \ .. or NUL)          throw …"forbidden path characters";
if (!std::isfinite(weight) || …)          throw …"must be finite numbers";
if (weight <= 0)                          throw …"Weight must be positive";
if (friction < 0)                         throw …"Friction coefficient cannot be negative";
if (accelForce <= 0 || brakeForce <= 0)   throw …;
if (departure == arrival)                 throw …"Departure and arrival must differ";
if (departureTime < 0 || stopDuration < 0) throw …;
```

The name check is a safety measure: the name becomes a file name, so `../` must never get through.

**GUI path:** `MainWindow` calls `InputHandler::parseNetworkFile` / `parseTrainFile` itself (`bonus/gui/MainWindow.cpp`, ~line 856) so the map shows instantly; `SimulationWorker::runSimulation` calls `loadData()` again on *Run*.

---

## 7. The Full Conversation (sequence diagram)

| Step in the diagram | Code |
|---|---|
| Options typed | `main` option loop, `src/main.cpp:154`+ (`--distance`, `--solo`, `--adaptive` are mutually exclusive; `--runs N`; `--graph`; `--animate`) |
| Load input | `InputHandler::loadData` |
| Create Simulation | `Simulation(network, trains, events, pathfinder, weightMode)` |
| Run | `sim.run()` (`main.cpp:297`) |
| Diary lines | `FileOutputObserver::onTrainStart / onTrainStep / onTrainEvent / onTrainFinish` (`src/FileOutputObserver/`) |
| Animation | `sim.setAnimCallback(...)` → `TerminalAnimDisplay::render` / GUI `tick` signal |
| Summary table | `OutputManager::printResult` (`OutputManager.cpp:107`) |
| Graph picture | `GraphExporter::exportDot` (after the run) |
| GUI final table | `SimulationWorker` emits `finished(results)` → `MainWindow` |

---

## 8. The Rules of the Game — where each one is enforced

**No passing; 200 m safe gap** — `Simulation::applyBlocking` (`Simulation.cpp:1213`), constant `MIN_SAFE_FOLLOWING_DISTANCE_M = 200.0` (`Simulation.hpp:192`). The order of trains on a track is the order they *entered* it, which never flips:

```cpp
if (behind->speed_ms > ahead->speed_ms) { behind->speed_ms = ahead->speed_ms; capped = true; }
double maxBehindPos = ahead->posOnSegment_m - MIN_SAFE_FOLLOWING_DISTANCE_M;
if (behind->posOnSegment_m > maxBehindPos) { behind->posOnSegment_m = std::max(maxBehindPos, 0.0); capped = true; }
```

**Events halt the train** — `holdTrain` (`Simulation.cpp:767`). A train held on the open track keeps its queue position (`heldBySegmentEvent`):

```cpp
void Simulation::holdTrain(TrainState &s, double delay, bool onSegment) {
    if (onSegment && delay > 0.0) s.heldBySegmentEvent = true;
    s.train->recordDelay(delay);
    s.stopTimer += delay;
    s.speed_ms = 0.0;
}
```

and in `run`, when the countdown ends: `if (!s.heldBySegmentEvent) s.segmentEntryTime = simTime;` — a normal dwell sends you to the back of the queue, an on-track hold does not.

**Rail events roll when a train enters the track** — `rollSegmentEvents` (`Simulation.cpp:778`); **station events when it arrives** — `handleSegmentTransition` (see §3).

**Rerouting (adaptive only)** — `considerReroute` (`Simulation.cpp:1016`) then `evaluateDetour` (`:1098`). Gates, in order:

```cpp
if (posInGroup == 0 || s.segsSinceReroute < REROUTE_COOLDOWN) return;  // leader, or too soon (3 tracks)
if (s.segmentIndex + 2 >= path.size()) return;                          // no choice left
/* find a leader whose route keeps sharing mine after the pivot */
if (myEta >= leaderEta) return;                                         // only if I am faster
/* budget: once on entry, once on change, once just before the pivot */
```

The decision is a replay of both futures:

```cpp
double etaStay   = replayEta(stay, &lead) + riskOf(path);
double etaDetour = replayEta(det,  &lead) + riskOf(newPath);
if (!(etaDetour + MIN_GAIN_SEC < etaStay)) return false;     // detour must win clearly
s.train->setPath(newPath);  s.segsSinceReroute = 0;  s.rerouteCount++;
```

The detour itself comes from `DijkstraPathfinding::findConstrainedPath`, which is Dijkstra forbidding the edges the leader would block.

**Honest input** — `TrainFactory::createTrain` and `InputHandler` (§6).

**Live changes** — `Simulation::enqueueMutation` (thread-safe; `:76`), applied at the start of each tick by `drainMutations` → `applyMutation` (`:89`, `:97`) into `applyAddNode / applyAddRail / applyAddEvent / applyAddTrain` (`:141`–`:176`). Rejections go through `_onMutationRejected`, e.g. after the last train has arrived:

```cpp
if (std::holds_alternative<AddTrainCommand>(mutation))
    _onMutationRejected("Simulation already completed; train not added");
```

The GUI guards the pointer to the running simulation with `std::mutex _liveSimMutex` (`bonus/gui/SimulationWorker.hpp`).

**Results** — each train's file is `<name>_<HHhMM>.result`, written by `FileOutputObserver` (`src/FileOutputObserver/FileOutputObserver.cpp`); the final numbers are collected into `TrainResult` at the end of `run()`.
