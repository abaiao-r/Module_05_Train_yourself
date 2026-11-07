# Bonus: Multiple Lines per Edge (`--lines`)

An edge between two stations can hold several **lines**. Trains may use a line in
either direction, but two trains travelling in opposite directions never occupy the
same line at the same time (no head-on collisions).

Everything here is opt-in. Without `--lines` (and without the GUI "Multi-line rails"
checkbox) the simulation behaves exactly as the mandatory project.

## Input

```
Rail <A> <B> <distance_km> <speed_kmh> [numLines]
```

- `numLines` is only accepted with `--lines`; it must be an integer >= 1 (default 1).
  Anything else fails with `file:line: Invalid line count 'X' (expected an integer >= 1)`.
- Lines share the edge's distance and speed. Rail events affect every line of the edge.
- Without `--lines` an extra token is rejected as before.

Example: `input/railNetworkPrintFolder/railNetworkParallelLines.txt`

```
make && ./bin/train_yourself input/railNetworkPrintFolder/railNetworkParallelLines.txt <trains> --lines
```

## Movement rules (lines mode)

1. A train at a station asks to **enter** the next edge. It owns no line while waiting.
2. Requests are resolved before any train moves in a tick, ordered by wait start and then
   train name, so two opposite trains can never both enter in the same tick.
3. A line is admissible if it is empty or used in the **same direction**. The train takes
   an empty line first, otherwise the same-direction line whose last train is furthest
   ahead; ties go to the lowest line id.
4. If every line carries opposite traffic the train waits at the station (status
   `Waiting`) until one clears.
5. **Starvation guard:** an opposite train that waited longer reserves the corridor, so
   later same-direction trains cannot keep joining ahead of it.
6. Trains brake to zero at every station in lines mode (no speed carry-over), because a
   train cannot wait at a station after already entering the next line.
7. Same-direction following on a line is unchanged (200 m safe distance, no mid-edge
   overtaking). A faster train overtakes by taking a different free line at admission.

## Pathfinding

Dijkstra only treats an edge as congested when its occupancy reaches the number of lines.
ETA replay for detour decisions does not cap a follower behind a leader on later edges that
have a spare line. It does not model queue waits behind opposite traffic.

## GUI

The toolbar checkbox "Multi-line rails" sets the same option. Saved and temporary network
files include line counts only when it is checked. Live `AddRailCommand` carries `numLines`,
ignored when the option is off. Scene rendering of line counts is not implemented.

## Tests

`tests/LinesTest.cpp` (admission, serialisation, overtaking, fairness, adversarial cases),
`tests/EndToEndTest.cpp` (golden default-mode output in `tests/golden/`), and parser tests
in `InputHandlerTest` / `RailNetworkTest`.
