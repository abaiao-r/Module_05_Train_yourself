# Demo inputs (manual testing)

Five network/train pairs. The full set of test fixtures lives in `tests/fixtures/`.
Run from the repo root after `make`. `N=input/railNetworkPrintFolder/railNetwork`, `T=input/trainPrintFolder/trainPrint`.

| # | Pair | Showcases |
|---|------|-----------|
| 1 | `Demo1Basic` | Normal run: cities, city and rail events, 3 trains. Try `--animate`, `--graph output/`, `--runs 1000`. |
| 2 | `Demo2Congestion` | Slow train ahead of a fast one: compare `--distance`, `--solo`, `--adaptive`. |
| 3 | `Demo3EventsReroute` | Risky segment plus a convoy: `--adaptive` reroutes around it. |
| 4 | `Demo4Lines` (bonus) | Needs `--lines`. Single-line vs 2-line edge with per-line speeds; opposite trains wait, never head-on. |
| 5 | `Demo5Detour` (bonus) | Needs `--lines`. Opposite trains on one single line; `--adaptive` makes one detour via `Via`. |

```
N=input/railNetworkPrintFolder/railNetworkDemo; T=input/trainPrintFolder/trainPrintDemo
./bin/train_yourself ${N}1Basic.txt ${T}1Basic.txt
./bin/train_yourself ${N}2Congestion.txt ${T}2Congestion.txt --adaptive
./bin/train_yourself ${N}3EventsReroute.txt ${T}3EventsReroute.txt --adaptive
./bin/train_yourself ${N}4Lines.txt ${T}4Lines.txt --lines
./bin/train_yourself ${N}5Detour.txt ${T}5Detour.txt --lines            # TrainBA waits
./bin/train_yourself ${N}5Detour.txt ${T}5Detour.txt --lines --adaptive # TrainBA detours
make run-gui   # GUI: use File -> Load, tick "Multi-line rails" for demos 4 and 5
```

Without `--lines`, demos 4 and 5 are rejected (the extra line-count token is invalid by design).
