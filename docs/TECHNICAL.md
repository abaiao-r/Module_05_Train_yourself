# Technical Documentation

> Back to [README](../README.md)

This is the central index for the project's technical documentation. Each topic lives in its own file for easy navigation.

---

## Documentation Map

| Document | Content |
|----------|---------|
| [BUSINESS_LOGIC.md](BUSINESS_LOGIC.md) | All 15 business rules — core (1-11) + bonus (12-15): network, trains, physics, events, output, graph export, multi-run, animation, GUI |
| [ARCHITECTURE.md](ARCHITECTURE.md) | Layered architecture overview, folder structure, data flow, dependency injection, Orthodox Canonical Form |
| [DESIGN_PATTERNS.md](DESIGN_PATTERNS.md) | Strategy, Factory, Observer patterns · Dependency Injection · Orthodox Canonical Form · Class Relationships (UML) · Design Principles (KISS, DRY, Encapsulation, SRP, Separation of Concerns, Interface Segregation) |
| [CLASS_REFERENCE.md](CLASS_REFERENCE.md) | Per-class API: Node, Edge, RailNetwork, Train, TrainStatus, TrainState, TrainResult, Event, TrainFactory, InputHandler, SimulationData, Simulation, OutputManager, FileOutputObserver, GraphExporter, TerminalAnimDisplay, GUI classes |
| [DIAGRAMS.md](DIAGRAMS.md) | All 7 UML diagrams: class, state machine, 2 activity, 3 sequence — with rendered PNGs and PlantUML sources |
| [BUILD_SYSTEM.md](BUILD_SYSTEM.md) | Makefile targets, dependency management (cross-platform), testing (498 tests / 13 suites), CI pipeline |

### Bonus Features

| Document | Content |
|----------|---------|
| [BONUS_MULTIRUN.md](BONUS_MULTIRUN.md) | `--runs N` flag — Monte Carlo analysis, statistical aggregation, output format |
| [BONUS_ANIMATE.md](BONUS_ANIMATE.md) | `--animate` flag — ANSI rendering, Unicode dashboard, ~20 FPS terminal UI |
| [BONUS_GUI.md](BONUS_GUI.md) | Qt 6 GUI — interactive graph editor, CRUD, live simulation, dashboard, dark theme |

---

## Quick Reference

### Design Patterns

- **Strategy** — `IPathfinding` → `DijkstraPathfinding` · [details](DESIGN_PATTERNS.md#strategy--pathfinding)
- **Factory** — `TrainFactory::createTrain()` · [details](DESIGN_PATTERNS.md#factory--train-creation)
- **Observer** — `ISimulationObserver` → `FileOutputObserver` · [details](DESIGN_PATTERNS.md#observer--simulation-output)
- **Dependency Injection** — constructor-based DI in `Simulation` · [details](DESIGN_PATTERNS.md#dependency-injection)
- **Design Principles** — KISS, DRY, Encapsulation, SRP, Separation of Concerns, Interface Segregation · [details](DESIGN_PATTERNS.md#design-principles)

### Key Diagrams

| Diagram | Preview |
|---------|---------|
| [Class diagram](DIAGRAMS.md#class-diagram) | Full project structure with bonus packages |
| [Train lifecycle](DIAGRAMS.md#state-machine--train-lifecycle) | Waiting → Running → Arrived / Aborted |
| [Simulation overview](DIAGRAMS.md#activity--simulation-overview) | End-to-end flow with all bonus branches |

### Build & Test

```bash
make            # build CLI
make test       # 498 tests
make bonus      # build GUI (Qt >= 6.3)
make setup      # auto-install all dependencies
```
