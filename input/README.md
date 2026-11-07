# Demo inputs

Ten network/train pairs, simple to complex. Run from the repo root after `make`.
Full test fixtures live in `tests/fixtures/`.

## Normal (no flags needed)

| # | Pair | Showcases | Command extras |
|---|------|-----------|----------------|
| 1 | `Normal1Simple` | One train on one rail | |
| 2 | `Normal2Following` | Same-direction trains keep a safe distance; different speed limits | `--animate` |
| 3 | `Normal3Events` | City and rail events, junctions, 3 trains | `--graph output/`, `--runs 1000` |
| 4 | `Normal4Congestion` | Slow train ahead of a fast one, fork with alternative | compare `--distance`, `--solo`, `--adaptive` |
| 5 | `Normal5Convoy` | Risky segment plus a convoy: reroutes | `--adaptive` |

## Bonus (`--lines`; also tick "Multi-line rails" in the GUI)

| # | Pair | Showcases |
|---|------|-----------|
| 1 | `Bonus1SingleLine` | One line, opposite trains: second waits, no head-on |
| 2 | `Bonus2TwoLines` | Two lines: opposite trains pass at once |
| 3 | `Bonus3Detour` | Single lines: TrainBA waits; with `--adaptive` it detours via `Via` |
| 4 | `Bonus4MixedSpeeds` | 1-line and 2-line edges with per-line speeds; fastest free line first |
| 5 | `Bonus5Complex` | Hub with mixed lines, opposite flows, an event: `--adaptive` avoids the 20 min delay |

```
N=input/railNetworkPrintFolder/railNetwork; T=input/trainPrintFolder/trainPrint
./bin/train_yourself ${N}Normal1Simple.txt ${T}Normal1Simple.txt
./bin/train_yourself ${N}Normal4Congestion.txt ${T}Normal4Congestion.txt --adaptive
./bin/train_yourself ${N}Bonus1SingleLine.txt ${T}Bonus1SingleLine.txt --lines
./bin/train_yourself ${N}Bonus5Complex.txt ${T}Bonus5Complex.txt --lines --adaptive
```

Bonus files are rejected without `--lines` (the line-count token is invalid by design).
