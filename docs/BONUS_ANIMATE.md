# Bonus — Terminal Animation (`--animate`)

A live full-screen terminal dashboard that visualises the simulation in real time using ANSI escape codes and Unicode box-drawing characters.

> Back to [README](../README.md) · See also [TECHNICAL.md](TECHNICAL.md)

---

## Usage

```bash
# Run with live animation
./bin/train_yourself network.txt trains.txt --animate

# Combine with time routing
./bin/train_yourself network.txt trains.txt --time --animate

# Combine with multi-run (animation plays first run only)
./bin/train_yourself network.txt trains.txt --animate --runs 100

# Shortcut
make run-animate
```

### CLI flag

```
--animate    Show live terminal animation of the simulation
```

When combined with `--runs N`, animation only plays during run 0; remaining runs proceed silently.

---

## What It Looks Like

```
╔══════════════════════════════════════════════════════════════════╗
║      Train Simulation — Live View                              ║
╠══════════════════════════════════════════════════════════════════╣
║  Clock: 14:35:22                                               ║
╠══════════════════════════════════════════════════════════════════╣
║  ● TrainAB1  (CityA → CityC)  [Running]                       ║
║      Segment: CityA → CityB                                    ║
║      Speed:    95.2 km/h  (Moving)                              ║
║      ████████████████████▓░░░░░░░░░░░░░░░░░░░  50%             ║
║      Elapsed: 00:25:22                                          ║
║  ─────────────────────────────────────────────────────────────  ║
║  ○ TrainBC2  (CityB → CityC)  [Waiting]                        ║
║      Departs at 15:00:00                                        ║
║  ─────────────────────────────────────────────────────────────  ║
║  ● TrainCA3  (CityC → CityA)  [Arrived]                        ║
║      Travel time: 01:12:45   Delay: 00:10:30                    ║
╚══════════════════════════════════════════════════════════════════╝
```

Updates at **~20 FPS** (~50 ms per frame). Cursor is hidden during animation and restored on cleanup.

---

## Architecture

### Class: `TerminalAnimDisplay`

| File | Location |
|------|----------|
| Header | `src/TerminalAnimDisplay/TerminalAnimDisplay.hpp` |
| Source | `src/TerminalAnimDisplay/TerminalAnimDisplay.cpp` |

**Design principle**: Pure rendering utility with **zero dependency on `Simulation`**. It only consumes a `std::vector<TrainState>` snapshot, making it fully decoupled and independently testable.

### Integration

In `main.cpp`, when `--animate` is active:

```cpp
TerminalAnimDisplay animDisplay;
sim.setAnimCallback([&](double t, const std::vector<TrainState>& s) {
    animDisplay.render(t, s);
});
// After sim.run():
animDisplay.cleanup();   // restore cursor
```

The `Simulation` engine calls the lambda **every physics tick** (every simulated second), passing `(simTime, trainStates)`.

### Class Members

| Member | Type | Purpose |
|--------|------|---------|
| `_firstFrame` | `bool` | On first call, sends `HIDE_CURSOR` + `CLEAR` |
| `_lastLineCount` | `int` | Lines drawn last frame (to erase stale content) |

### Public Methods

| Method | Description |
|--------|-------------|
| `render(simTime, states)` | Build and display one frame |
| `cleanup()` | Restore cursor visibility |

---

## Rendering Algorithm

Each call to `render()` follows this sequence:

```
1. First frame? → hide cursor, clear terminal
2. Move cursor to HOME (top-left) — overwrite previous frame in-place
3. Draw top border:      ╔═══════════════════════════════╗
4. Draw header:          ║  Train Simulation — Live View ║
5. Draw clock:           ║  Clock: HH:MM:SS              ║
6. For each TrainState:
   ├── Status icon:  ● green (arrived) │ ● yellow (running) │ ○ dim (waiting)
   ├── Name, ID, route (From → To)
   ├── Status tag: [Arrived] / [Waiting] / [Running] / [Stopped]
   ├── If waiting:  departure time
   ├── If arrived:  travel time + delay
   ├── If running:  segment, speed (km/h), action, progress bar, elapsed
   └── Divider: ────────────
7. Draw bottom border:   ╚═══════════════════════════════╝
8. Erase leftover lines from previous (longer) frame
9. Flush stdout
10. usleep(50000)  →  ~50 ms delay → ~20 FPS
```

### Progress Bar

Built with Unicode block characters:

| Character | Meaning |
|-----------|---------|
| `█` | Filled (completed portion) |
| `▓` | Edge (current position) |
| `░` | Empty (remaining portion) |

Width: 40 characters. Shows percentage next to the bar.

### ANSI Escape Codes Used

| Code | Purpose |
|------|---------|
| `\033[2J` | Clear entire screen |
| `\033[H` | Move cursor to home (top-left) |
| `\033[?25l` | Hide cursor |
| `\033[?25h` | Show cursor |
| `\033[1m` | Bold |
| `\033[2m` | Dim |
| `\033[0m` | Reset all attributes |
| `\033[31m` – `\033[37m` | Red, green, yellow, cyan, white |

---

## Helper Functions

| Function | Signature | Description |
|----------|-----------|-------------|
| `fmtTime` | `string fmtTime(double seconds)` | Format seconds → `"HH:MM:SS"` |
| `progressBar` | `string progressBar(double fraction, int width)` | Unicode block bar |
| `speedStr` | `string speedStr(double speed_ms)` | `"  123.4 km/h"` |
| `visLen` | `size_t visLen(const string& s)` | Visible width, stripping ANSI escapes, counting UTF-8 |
| `boxLine` | `string boxLine(const string& content)` | Pad to 62 visible chars, wrap in `║…║` |

---

## Platform Compatibility

| Platform | Status | Notes |
|----------|--------|-------|
| macOS Terminal | ✅ | Full support |
| Linux terminal | ✅ | Full support (any terminal with ANSI + UTF-8) |
| WSL | ✅ | Works in Windows Terminal |
| Windows cmd.exe | ⚠️ | ANSI codes not supported in legacy console |

Requires a terminal that supports:
- ANSI CSI escape sequences (colours, cursor movement)
- UTF-8 encoding (box-drawing characters, block elements)
