# Demo inputs

One normal pair and eleven bonus pairs. Run from the repo root after `make`.
Full test fixtures live in `tests/fixtures/`.

## Normal (no flags needed)

| Pair | Showcases | Command extras |
|------|-----------|----------------|
| `Normal3Events` | City and rail events, junctions, 3 trains | `--graph output/`, `--runs 1000` |

## Bonus (`--lines`; also tick "Multi-line rails" in the GUI)

| # | Pair | Showcases | What you should see |
|---|------|-----------|---------------------|
| 01 | `Bonus01HeadOn` | One line, opposite trains | `West` waits at B until `East` has left the line |
| 02 | `Bonus02TwoLines` | Two lines, opposite trains | both enter at once, on different lines |
| 03 | `Bonus03Following` | One line, same direction | `Fast` stays behind `Slow` |
| 04 | `Bonus04Overtake` | Two lines, same direction | `Fast` takes the spare line and arrives first |
| 05 | `Bonus05LineSpeeds` | Per-line speeds (60 / 160 km/h) | `First` takes the fast line, `Second` the slow one |
| 06 | `Bonus06LineEvent` | Event on L1 only | the train takes L2 (`--solo` and `--adaptive`) |
| 07 | `Bonus07Detour` | Waiting train detours | `Back` waits with `--solo`, goes via `C` with `--adaptive` |
| 08 | `Bonus08Fairness` | Starvation guard | `Ret` crosses between the `T` trains, not after them all |
| 09 | `Bonus09Hub` | Hub, mixed lines, opposite flows, an event | `--adaptive` avoids the 20 min strike |
| 10 | `Bonus10RushHour` | 3-, 2- and 1-line rails, line and rail events | busiest case |
| 11 | `Bonus11LisbonMetro` | Lisbon metro: 4 lines, interchanges, real station names | drawn with the real geography (see below) |

Events are random, so event cases (06, 09, 10, 11) vary run to run; use `--runs N` for averages.

```
N=input/railNetworkPrintFolder/railNetwork; T=input/trainPrintFolder/trainPrint
./bin/train_yourself ${N}Normal3Events.txt ${T}Normal3Events.txt
./bin/train_yourself ${N}Bonus01HeadOn.txt ${T}Bonus01HeadOn.txt --lines
./bin/train_yourself ${N}Bonus07Detour.txt ${T}Bonus07Detour.txt --lines --adaptive
./bin/train_yourself ${N}Bonus11LisbonMetro.txt ${T}Bonus11LisbonMetro.txt --lines --adaptive
```

Bonus files are rejected without `--lines` (the line-count token is invalid by design).

## Drawing layout (GUI only)

A network file may have a sidecar `railNetwork<Name>.layout` with one
`Station x y` per line (`#` comments). The GUI uses it instead of the
automatic layout when it places every station; the CLI ignores it.
`Bonus11LisbonMetro.layout` follows the shape of the real network.
