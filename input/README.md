# Demo inputs

One normal pair and fourteen bonus pairs. Run from the repo root after `make`.
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
| 12 | `Bonus12WaitOrGo` | Wait or go | `Go` waits ~5 min for the busy 160 km/h line instead of taking the free 30 km/h one (arrives 08:16 instead of 08:43) |
| 13 | `Bonus13Stress` | 300 nodes, 595 rails (1-4 lines, per-line speeds), 52 events, 3000 varied trains; regenerate with `scripts/gen_stress.py [seed cols rows trains]` | `--lines` (about 40 s); try `--adaptive` or `--planner cbs` |
| 14 | `Bonus14GreaterLisbon` | 71 stations across CP Urbanos and Fertagus commuter corridors | OSM-geographic layout; includes Sintra, Azambuja, Cascais, Sado and Fertagus services |

Events are random, so event cases (06, 09, 10, 11) vary run to run; use `--runs N` for averages.

```
N=input/railNetworkPrintFolder/railNetwork; T=input/trainPrintFolder/trainPrint
./bin/train_yourself ${N}Normal3Events.txt ${T}Normal3Events.txt
./bin/train_yourself ${N}Bonus01HeadOn.txt ${T}Bonus01HeadOn.txt --lines
./bin/train_yourself ${N}Bonus07Detour.txt ${T}Bonus07Detour.txt --lines --adaptive
./bin/train_yourself ${N}Bonus11LisbonMetro.txt ${T}Bonus11LisbonMetro.txt --lines --adaptive
./bin/train_yourself ${N}Bonus14GreaterLisbon.txt ${T}Bonus14GreaterLisbon.txt --lines --adaptive
```

Bonus files are rejected without `--lines` (the line-count token is invalid by design).

## Video levels (`input/video/`, `--lines`)

One invented city that gains one idea per level, built for screen recordings
(big labels, fixed `.layout` positions). Regenerate with
`scripts/gen_video_scenarios.py`.

| Pair | Idea | What you should see |
|------|------|---------------------|
| `L1Physics` | One train, one rail | `Blue` accelerates, cruises and brakes into Old_Town (08:03:53) |
| `L2Route` | Short-slow vs long-fast route | `--distance` goes via Old_Town (18 min 40 s); `--solo` via Ridge (11 min 24 s) |
| `L3HeadOn` / `L3TwoLines` | Opposite trains, 1 vs 2 lines | `Blue` waits for `Red` on one line (08:15); no wait with two (08:11) |
| `L4Stuck` / `L4Overtake` | Express behind freight | `Express` arrives 08:33 when stuck, 08:13 when it can overtake |
| `L5Bridge` | Certain 30 min closure on the fast route | `--solo` waits (08:41); `--adaptive` goes via Old_Town (08:19) |
| `L6RushHour` | 30 trains, random events | compare planners with `--compare --runs 100` |
| `L7GreaterLisbon` | Lisbon-inspired model: Metro (50 stations) + CP/Fertagus (71 stations), separate track systems, 96 trains | all trains arrive (`--lines --adaptive`); planners within 1% (`--compare --runs 40`) |

## Drawing layout (GUI only)

A network file may have a sidecar `railNetwork<Name>.layout` with one
`Station x y` per line (`#` comments). The GUI uses it instead of the
automatic layout when it places every station; the CLI ignores it.
`Bonus11LisbonMetro.layout` follows the shape of the real network.

`Bonus14GreaterLisbon.layout` uses OpenStreetMap station coordinates for the
CP Urbanos and Fertagus heavy-rail network. The Metro remains a separate sample
(`Bonus11LisbonMetro`): Metro tracks are not physically connected to the
commuter-rail tracks, so combining them would let simulated trains use routes
that do not exist.

The Greater Lisbon data represents real stations, corridors, and rolling-stock
series, not live train tracking. Departures and simulator vehicle parameters
are representative examples, not current timetables or manufacturer-verified
physics. Cascais segment lengths use published chainage; other segment lengths
are estimates derived as 1.10 times the OSM station-to-station geodesic
distance. Track counts and speeds are simplified for this simulation; the
simulator applies its configured stop duration at intermediate stations and
does not model each service's skip-stop pattern.

Sources: [CP Urbanos de Lisboa](https://www.cp.pt/passageiros/en/how-to-travel/lisbon-urban-trains);
[Sintra service](https://pt.wikipedia.org/wiki/Linha_de_Sintra_%28CP%29) and
[Azambuja service](https://pt.wikipedia.org/wiki/Linha_da_Azambuja),
[Cascais station chainage](https://pt.wikipedia.org/wiki/Linha_de_Cascais),
[Sado service](https://pt.wikipedia.org/wiki/Linha_do_Sado), and the
[Fertagus network map](https://www.fertagus.pt/pt/viajar/mapa-de-rede).
Station positions are from [OpenStreetMap](https://www.openstreetmap.org/copyright)
(© OpenStreetMap contributors), licensed under the ODbL. Rolling-stock references: [CP Series 2300](https://pt.wikipedia.org/wiki/S%C3%A9rie_2300_da_CP),
[CP Series 3150](https://pt.wikipedia.org/wiki/S%C3%A9rie_3150_da_CP), and
[Series 3500](https://pt.wikipedia.org/wiki/S%C3%A9rie_3500_da_CP).
