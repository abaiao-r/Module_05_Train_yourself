# Bonus: cooperative fleet routing (`--planner cooperative`)

Epic #154. Opt-in; the default behaviour and all golden outputs are unchanged.

## The problem: every train is selfish

`IPathfinding` answers "what is the best path for ONE train?". When trains
share single-track lines, each train's best path can hurt the others
(the *price of anarchy*):

```
        B
      /   \          T1 and T2 both pick A-B-D (shortest).
    A       D        On ONE line per rail they queue / wait for each other.
      \   /          If T2 took A-C-D instead (+36 s), nobody waits.
        C
```

## Architecture (Strategy pattern, two levels)

```
 Simulation --owns--> IRoutePlanner            (fleet level: ALL trains)
                         |- IndependentPlanner   one Dijkstra per train (default)
                         |- PrioritizedPlanner   trains planned one by one
                         |- KeepPathsPlanner     replays given routes (dry runs)
                         '- RefinedPlanner       decorator: local search
                                 |
                                 v uses
                      IPathfinding (Dijkstra)   (train level: ONE train)
```

* `ReservationTable` - which (lane, direction) is booked in which time window.
  Opposite directions on one lane conflict, same direction does not.
* `CooperativeCost` - turns reservations into a *toll* in seconds per directed
  segment: the exact wait for opposite traffic, or `FOLLOW_PENALTY_SEC = 20`
  per same-way train. A free parallel lane costs 0. The toll is added to the
  existing event risk, so Dijkstra itself is untouched.
* `PrioritizedPlanner` - order by departure (ties by name). Each train is
  routed with the toll of the trains before it, then reserves its own path.
* `RefinedPlanner(inner, baseline)` - starts from the better of
  {inner, baseline}, then tries, per train, alternatives that forbid one rail
  of its current route. A move is kept only if the dry-run total of the whole
  fleet (plus expected random-event delay) drops by at least 1 s. Max 3 passes.
  The baseline wins ties, so it can only help in the dry-run model.

## Usage

```
./bin/train_yourself NET TRN --lines --planner cooperative
./bin/train_yourself NET TRN --lines --compare --runs 40   # 4 columns
```

`--planner independent|cooperative`; an unknown value is an error.
`--planner` with `--compare` is an error (compare already has a Cooperative
column). Cooperative implies Adaptive dynamics unless a mode is given.
The GUI has a "Cooperative" entry in the Path combo (single/multi run;
"Compare all" still shows the original three modes).

Without `--lines` there are no head-on conflicts, so there is little for the
planner to coordinate.

## Benchmark (`--compare --runs 40 --lines`, sum of average travel / delay)

| Scenario | Distance | Solo | Adaptive | Cooperative |
|---|---|---|---|---|
| 07Detour (delay) | 6:33 | 6:33 | 3:46 | **3:46** |
| 09Hub (delay) | 23:34 | 21:34 | **10:51** | **10:51** |
| 10RushHour (delay) | 38:55 | 43:55 | **30:35** | 35:18 |
| 12WaitOrGo (delay) | 5:23 | 5:23 | 5:23 | 5:23 |
| 11Lisbon (delay) | 40:24 | 45:16 | **35:39** | 43:24 |

Random events move totals by +-3-5 min between runs, so small gaps are noise.

## Honest conclusion

Cooperative planning **does not beat reactive Adaptive** on these fixtures
(it ties on Detour/Hub/WaitOrGo and is worse or equal on RushHour/Lisbon).
Why:

1. Random events (up to 20 min) dominate; Adaptive reroutes after seeing them,
   a pre-computed plan cannot.
2. Lisbon is nearly a tree: there are few alternative routes to share.
3. The planner's timing model (free-flow timeline plus an event-free dry run)
   ignores acceleration and dwell, so it under-estimates congestion.
4. Same-direction following is cheap in the real simulator (about +65 s for
   two trains), so there is little traffic to avoid.

Where it helps: the diamond unit test (1505 s independent vs 1482 s
cooperative) - static, event-free, head-on-prone networks.

## Limits / not done

* #159 (re-plan not-yet-departed trains when an event fires): not implemented.
  Live rerouting is already done by Adaptive (`considerReroute`).
* #162 (conflict-based search): not implemented, left open.
* Live-added trains (`addTrain`) use plain pathfinding, not the planner.
* Planning costs about 1.5 s per run on Lisbon (dry-run evaluations).

## References

Prioritized planning and Conflict-Based Search (multi-agent path finding),
Wardrop equilibrium / price of anarchy, rolling-horizon re-planning.
