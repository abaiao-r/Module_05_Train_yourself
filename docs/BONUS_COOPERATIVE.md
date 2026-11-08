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
                         |- CbsPlanner           conflict-based search (--planner cbs)
                         |- KeepPathsPlanner     internal: replays given routes in dry runs
                         '- RefinedPlanner       decorator: local search
                                 |
                                 v uses
                      IPathfinding (Dijkstra)   (train level: ONE train)
```

* `ReservationTable` - which (lane, direction) is booked in which time window.
  Opposite directions on one lane conflict, same direction does not.
* `CooperativeCost` - turns reservations into a *toll* in seconds per directed
  segment: the estimated wait for opposite traffic (free-flow model), or `FOLLOW_PENALTY_SEC = 20`
  per same-way train. A free parallel lane costs 0. The toll is added to the
  existing event risk, so Dijkstra itself is untouched.
* `PrioritizedPlanner` - order by departure (ties by name). Each train is
  routed with the toll of the trains before it, then reserves its own path.
* `CbsPlanner` (`--planner cbs`) - conflict-based search, lite. Plans every
  train alone, finds the first head-on conflict (opposite directions on a
  single-line rail in overlapping free-flow windows), branches by forbidding
  that rail to one of the two trains (the lower train name is branched first), re-plans only that
  train and expands best-first by fleet cost. The prioritized solution is the
  fallback incumbent, so the plan is never costlier than `PrioritizedPlanner`
  *under CBS's own cost model* (the dry run, or the free-flow timeline); this
  says nothing about real simulated time. Budget 200 expansions; when exhausted it prints
  `CBS: budget of N expansions reached, using the best solution found`.
* `ReplanPolicy` - after a rail event, trains that have **not departed yet**
  may switch route once per run, only if their own gain is >= 30 s and the
  other trains lose no more than that gain (checked with the dry run).
  Trains already under way keep their route; Adaptive handles them live.
  Active for `cooperative` and `cbs` (not `independent`); a `[replan]` log line appears only when a switch really happens.
* `RefinedPlanner(inner, baseline)` - starts from the better of
  {inner, baseline}, then tries, per train, alternatives that forbid one rail
  of its current route. A move is kept only if the dry-run total of the whole
  fleet (plus expected random-event delay) drops by at least 1 s. Max 3 passes.
  The baseline wins ties, so it can only help in the dry-run model. Its search is
  limited to alternatives that avoid one rail of the current route.

## Usage

```
./bin/train_yourself NET TRN --lines --planner cooperative
./bin/train_yourself NET TRN --lines --compare --runs 40   # 5 columns
```

`--planner independent|cooperative|cbs`; an unknown value is an error.
`--planner` with `--compare` is an error (compare already has Cooperative and CBS
columns). Cooperative implies Adaptive dynamics unless a mode is given.
In the GUI two combos sit side by side:

```
 Path:    [Distance | Solo | Adaptive | Compare all]   how ONE train weighs routes
 Planner: [Independent | Cooperative | CBS]            who decides the routes
```

Choosing Cooperative or CBS forces Path = Adaptive (Path greys out);
"Compare all" runs all five variants (Distance, Solo, Adaptive, Cooperative,
CBS) side by side and greys out Planner. Both combos have tooltips.

Without `--lines` there are no head-on conflicts, so there is little for the
planner to coordinate.

## Benchmark (`--compare --runs 40 --lines`, sum of average travel / delay)

| Scenario | Distance | Solo | Adaptive | Cooperative | CBS |
|---|---|---|---|---|---|
| 07Detour (delay) | 6:33 | 6:33 | 3:46 | 3:46 | 0:00 |
| 09Hub (delay) | 19:34 | 21:34 | 10:51 | 10:51 | 10:51 |
| 10RushHour (delay) | 42:55 | 39:10 | 28:20 | 40:48 | 41:46 |
| 12WaitOrGo (delay) | 5:23 | 5:23 | 5:23 | 5:23 | 5:23 |
| 11Lisbon (delay) | 39:39 | 42:01 | 43:54 | 40:16 | 44:01 |

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

* CBS-lite only resolves head-on conflicts on single-line rails, in the
  free-flow model; there is no fixture where it strictly beats Prioritized, so
  a mutation that branches on only one of the two trains is not caught by a
  test.
* Event re-planning only touches trains that have not departed; a train
  re-planned once is never re-planned again (no flip-flop by construction).
* Live-added trains (`addTrain`) use plain pathfinding, not the planner.
* Planning costs about 1.5 s per run on Lisbon (dry-run evaluations).
* CBS 40-run delay sums (Hub 10:51, RushHour 35:31, Lisbon 42:53) are within
  event noise of cooperative; it is not a measured improvement.

## References

Prioritized planning and Conflict-Based Search (multi-agent path finding),
Wardrop equilibrium / price of anarchy, rolling-horizon re-planning.
