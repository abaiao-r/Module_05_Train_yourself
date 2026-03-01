# Bonus — Qt GUI

An interactive graphical application built with Qt 6 for creating, editing, and simulating rail networks visually.

> Back to [README](../README.md) · See also [TECHNICAL.md](TECHNICAL.md)

---

## What Is Qt?

**Qt** (pronounced "cute") is an open-source, cross-platform C++ framework for building graphical user interfaces and applications. It is widely used in industry (KDE, VLC, Maya, Autodesk, Tesla dashboards) and academia.

### Why Qt for this project?

| Reason | Detail |
|--------|--------|
| **Cross-platform** | Same codebase compiles on macOS, Linux, and Windows — no platform-specific UI code |
| **C++ native** | Integrates directly with the existing C++17 simulation engine — no language bridge needed |
| **Rich widget set** | `QGraphicsScene` provides an interactive 2D canvas ideal for graph visualization with built-in drag, zoom, and pan |
| **Thread support** | `QThread` + signals/slots enable safe background simulation without freezing the UI |
| **Signal/slot system** | Qt's signal-slot mechanism provides type-safe, decoupled communication between components (similar to the Observer pattern already used in the CLI) |

### Qt modules used

| Module | Purpose |
|--------|---------|
| `QtCore` | Base types, threading (`QThread`), meta-type system |
| `QtGui` | Low-level graphics, colours, fonts |
| `QtWidgets` | High-level UI: `QMainWindow`, `QGraphicsView`, `QSplitter`, `QPushButton`, `QSlider`, `QTextEdit`, `QDockWidget`, dialogs |

### Key Qt concepts in this project

| Concept | Where used | Explanation |
|---------|-----------|-------------|
| **Signals & Slots** | `SimulationWorker` ↔ `MainWindow` | Thread-safe callbacks: worker emits `tick()/finished()/error()`, main window slots receive them on the UI thread |
| **QGraphicsScene** | `NetworkScene` | 2D scene graph — each node/edge is a graphics item that can be moved, styled, and hit-tested |
| **Worker Object Pattern** | `SimulationWorker` | Object is `moveToThread()` so its slots run on the worker thread, keeping the UI responsive |
| **Meta-type registration** | `Q_DECLARE_METATYPE(QVector<TrainSnapshot>)` | Allows custom structs to be passed across thread boundaries via queued signal/slot connections |
| **Stylesheets** | `MainWindow::STYLESHEET` | CSS-like syntax for theming all widgets (dark theme) without subclassing |
| **qmake** | `bonus.pro` | Build system that generates platform-appropriate Makefiles from a `.pro` project file |

---

## Usage

### Build & Launch

```bash
# Check / install dependencies (compiler + Qt >= 6.3)
make setup

# Build the GUI
make bonus

# Build & launch
make run-gui
```

On **macOS**, `make run-gui` opens `bin/TrainGUI.app`. On **Linux/WSL**, it runs `./bin/TrainGUI` directly.

### Requirements

- Everything needed for the CLI build (C++17 compiler, make)
- **Qt 6** (`core`, `gui`, `widgets` modules)
- qmake (auto-detected by the Makefile)

---

## Application Overview

The GUI is a single-window application with four main areas:

```
┌─────────────────────────────────────────────────────────────────┐
│  Toolbar: [Run] [Stop]  Speed: ◄──●──► 1.0×  [Fit Graph]        │
├────────────────────────────────────┬──────────┬─────────────────┤
│                                    │ Live     │ Network         │
│         Interactive Graph          │ Dashboard│ Elements        │
│         (drag nodes, see trains)   │ (box UI) │ (sidebar dock)  │
│                                    │          │ ┌─────────────┐ │
│                                    │          │ │ Presets      ││
│                                    │          │ │ Nodes  [+][-]││
│                                    │          │ │ Edges  [+][-]││
│                                    │          │ │ Trains [+][-]││
│                                    │          │ │ Events [+][-]││
│                                    │          │ └─────────────┘ │
├────────────────────────────────────┴──────────┤                 │
│  Log Panel (colour-coded messages)            │                 │
└───────────────────────────────────────────────┴─────────────────┘
```

---

## Architecture

### Source Files

| File | Class | Purpose |
|------|-------|---------|
| `bonus/gui/main.cpp` | — | Qt app entry point, project root detection |
| `bonus/gui/MainWindow.cpp/hpp` | `MainWindow` | Main window: menus, toolbar, CRUD, simulation control |
| `bonus/gui/NetworkScene.cpp/hpp` | `NetworkScene` | Interactive QGraphicsScene with draggable nodes |
| `bonus/gui/SimulationWorker.cpp/hpp` | `SimulationWorker` | Background-threaded simulation runner |
| `bonus/gui/SimDashboard.cpp/hpp` | `SimDashboard` | Live HTML dashboard widget |
| `bonus/gui/bonus.pro` | — | qmake project file |

### Build System

The `bonus.pro` file uses qmake and includes all shared simulation sources from `../../src/` plus the GUI-specific sources. Output goes to `../../bin/TrainGUI`.

---

## Features

### 1. Interactive Graph Editor

The network is visualised as an interactive node-edge graph in a `QGraphicsScene`.

#### Nodes
- **Draggable** — click and drag any node; connected edges and labels follow automatically
- **Hover effects** — visual feedback on mouse over
- **Force-directed layout** — automatic positioning via `layoutForceDirected()` after loading a network

#### Edges
- Drawn as lines between node centres
- Labels show distance and speed limit
- Automatically reposition when nodes are dragged

#### Train Animation
During simulation, train positions are shown as **coloured dots** on the graph:
- Each train gets a distinct colour from a built-in palette
- Position is interpolated along edges based on segment progress
- **Trail lines** show the path each train has travelled

#### Navigation
- **Zoom** — mouse wheel (anchored under cursor)
- **Pan** — click and drag on empty space
- **Fit Graph** — toolbar button to auto-fit the entire network in the viewport

### 2. Full CRUD Operations

All four entity types support Create, Edit, and Delete through dialog-based UI:

| Entity | Create | Edit | Delete |
|--------|--------|------|--------|
| **Node** | Name input dialog | Rename dialog | Remove + cascade edges |
| **Edge** | From/To combo + distance/speed inputs | Update distance/speed | Remove connection |
| **Train** | All 9 fields (name, weight, friction, forces, stations, time, stop) | Modify any field | Remove train |
| **Event** | Name, probability, duration, target node(s) | Modify any field | Remove event |

Each entity type has a dedicated **list widget** in the sidebar with **Add**, **Edit**, and **Delete** buttons.

### 3. File I/O

#### Loading
| Action | Description |
|--------|-------------|
| Load Network | Open a `.txt` network file (File menu or shortcut) |
| Load Trains | Open a `.txt` train file |
| Import All | Load both network + train files at once |

Loading uses the same `InputHandler` parser as the CLI, ensuring format compatibility.

#### Saving
| Action | Description |
|--------|-------------|
| Save Network | Write current network to `.txt` |
| Save Trains | Write current trains to `.txt` |
| Export All | Save both files at once |
| Export DOT | Export Graphviz `.dot` file for external graph rendering |

#### Preset Library

Two combo boxes in the sidebar auto-populate by scanning:
- `input/railNetworkPrintFolder/` — network presets
- `input/trainPrintFolder/` — train presets

Select a preset from the dropdown to load it instantly. This makes it easy to test with the bundled sample networks (France, Portugal, Lisbon Metro, Paris Metro, Europe, etc.).

### 4. Simulation Runner

#### Toolbar Controls

| Control | Description |
|---------|-------------|
| **Run** (green) | Starts the simulation. Writes temporary files from in-memory model, launches background worker |
| **Stop** (red) | Aborts a running simulation cleanly via atomic flag |
| **Speed slider** | Logarithmic scale from 0.1× to 10×. Centre (default) = 1.0× real-time |
| **Speed label** | Shows current multiplier (e.g., "2.5×") |
| **Fit Graph** (blue) | Auto-fits the graph to fill the viewport |

#### Speed Control

The slider uses a **logarithmic mapping** for natural feel:

| Slider position | Multiplier | Effect |
|-----------------|-----------|--------|
| 0 (leftmost) | 0.1× | 10× slower than real-time |
| 10 (centre) | 1.0× | Real-time |
| 20 (rightmost) | 10× | 10× faster than real-time |

Formula: `multiplier = 10^((sliderValue - 10) / 10)`

Frame rate is constant at ~20 FPS regardless of speed setting.

#### Background Thread

Simulations run in a **separate `QThread`** via the Worker Object pattern:
- `SimulationWorker` lives on a dedicated thread
- Communicates via Qt signals: `tick()`, `finished()`, `error()`
- **Thread-safe controls**: `std::atomic<double>` for speed, `std::atomic<bool>` for stop
- Uses `TrainSnapshot` (a value-type struct) for safe cross-thread data transfer

#### Stop Mechanism

The stop button sets an atomic flag. The simulation callback checks this flag every frame:
- If set, throws a `SimulationStopException`
- The worker catches it cleanly, emits `finished()`, and resets state
- Run/Stop buttons toggle enabled state appropriately

### 5. Live Dashboard

The `SimDashboard` widget (right panel) mirrors the terminal `--animate` dashboard, rendered as **HTML** in a `QTextEdit`:

- Unicode box-drawing borders (`╔═╗ ║ ╚═╝`)
- Per-train status with coloured indicators
- Progress bars with Unicode block characters
- Speed, segment, elapsed time, delay information
- Auto-scrolls to keep current content visible

#### Dashboard States

| State | Display |
|-------|---------|
| **Idle** | Welcome message |
| **Running** | Live updating train status (every tick) |
| **Finished** | Final results table |

### 6. Logging

A colour-coded log panel at the bottom records all actions:

| Log Level | Colour | Examples |
|-----------|--------|----------|
| Info | Blue | "Loaded network: france.txt" |
| Success | Green | "Simulation completed successfully" |
| Error | Red | "No trains defined — cannot run" |

### 7. Validation & Safety

- **Edit guard**: All CRUD operations check `guardSimRunning()` — if a simulation is active, a warning is shown asking the user to stop it first
- **Deferred validation**: Loading network/train files only parses (no cross-validation). Full validation (station existence, route validity) happens at Run time via the simulation worker
- **Node name validation**: Checked on create/edit to prevent empty or invalid names

---

## Class Details

### MainWindow

The central `QMainWindow` subclass. Owns all model data and UI widgets.

**Model data:**
- `RailNetwork _network` — the graph model
- `vector<unique_ptr<Train>> _trainDefs` — train definitions
- `vector<Event> _eventDefs` — event definitions
- `_useTimeWeight` — distance vs. time optimisation toggle

**Layout builders:**
| Method | Creates |
|--------|---------|
| `buildMenus()` | File menu (Load/Save/Import/Export) |
| `buildToolbar()` | Run, Stop, Speed slider, Fit Graph |
| `buildCentralWidget()` | vSplitter: (hSplitter: graph + dashboard) + log |
| `buildSidebar()` | Dock widget with presets + entity lists |

**Temporary file generation:**
Before running, `writeTemporaryNetworkFile()` and `writeTemporaryTrainFile()` serialise the in-memory model to temp files, which are passed to the `SimulationWorker`. This reuses the exact same `InputHandler` parsing path as the CLI.

### NetworkScene

A `QGraphicsScene` subclass with custom `DraggableNode` items.

**Key features:**
- `DraggableNode` (inner class) — `QGraphicsEllipseItem` subclass with `itemChange()` override for drag notifications and hover events
- `NodeVis` / `EdgeVis` structs — track graphics items + metadata
- `layoutForceDirected()` — automatic graph layout
- `updateTrains(simTime, snapshots)` — interpolate train positions on edges, draw coloured dots + trail lines
- `edgePosition(from, to, fraction)` — compute a point along an edge at a given fraction

### SimulationWorker

Worker Object pattern for background simulation.

**Thread-safe interface:**
| Method | Type | Description |
|--------|------|-------------|
| `setSpeedMultiplier(double)` | atomic | Set simulation speed |
| `requestStop()` | atomic | Signal the worker to stop |
| `isStopRequested()` | atomic | Check stop flag |

**Signals:**
| Signal | Payload | When |
|--------|---------|------|
| `tick` | `(double simTime, QVector<TrainSnapshot>)` | Every physics tick |
| `finished` | `QStringList results` | All trains arrived (or stopped) |
| `error` | `QString message` | Exception during simulation |

### SimDashboard

A `QTextEdit` subclass (read-only, monospace) rendering the live dashboard.

**Key helpers** (same logic as `TerminalAnimDisplay`):
- `visLen(html)` — visible character count, stripping HTML tags and entities
- `visTruncate(html, maxLen)` — truncate at visible-character boundary preserving HTML tags
- `boxRow(content)` — auto-pad and wrap in `║…║` borders
- `hRule(left, right, width)` — generate `═` horizontal rules

Box width: `BOX_INNER = 56` characters.

---

## Platform Compatibility

| Platform | Status | Notes |
|----------|--------|-------|
| macOS | ✅ | Tested with Qt 6.10.2 via Homebrew |
| Linux | ✅ | Qt 6 from system packages |
| WSL | ✅ | Requires X server or WSLg for GUI display |

### WSL-Specific Notes

To run the GUI on WSL, you need either:
- **WSLg** (Windows 11) — GUI apps work out of the box
- **X server** (Windows 10) — install VcXsrv or similar, set `DISPLAY=:0`

---

## Dark Theme

The entire GUI uses a custom dark colour scheme defined via Qt stylesheets:

| Element | Colour |
|---------|--------|
| Background | `#0f172a` (deep navy) |
| Panels | `#1e293b` (dark slate) |
| Borders | `#334155` (medium slate) |
| Text | `#e2e8f0` (light grey) |
| Accent (Run) | `#059669` (emerald green) |
| Accent (Stop) | `#dc2626` (red) |
| Accent (Speed) | `#06b6d4` (cyan) |
| Accent (Fit) | `#3b82f6` (blue) |
| Success logs | `#10b981` (green) |
| Error logs | `#f87171` (red) |
| Info logs | `#60a5fa` (blue) |
