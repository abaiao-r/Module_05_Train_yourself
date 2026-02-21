# Train Yourself — Rail Network Simulator

A train simulation engine that models rail traffic across a configurable network of stations and tracks. Trains find optimal routes, travel in real time, and face random disruptions — producing a full arrival report with delays.

## Why It Matters

Rail logistics is a billion-euro industry where even small scheduling improvements save fuel, time, and money. This simulator lets you:

- **Model any rail network** — define stations, tracks, distances, and speed limits in a plain text file.
- **Simulate realistic traffic** — multiple trains depart at different times, each finding the shortest route automatically.
- **Inject random events** — disruptions like riots or passenger discomfort trigger probabilistically, causing delays at specific stations.
- **Analyse results** — every train's journey is logged with departure, route, events, arrival time, and accumulated delay.

It serves as both a learning tool for software engineering (OOP, design patterns, testing, CI) and a foundation that could be extended into a real planning aid.

## Quick Start

### Requirements

- A C++17 compiler (`c++`, `g++`, or `clang++`)
- `make`

### Build & Run

```bash
# Build the binary
make

# Run with the provided sample data
make run

# Or run manually with your own files
./bin/Train path/to/network.txt path/to/trains.txt
```

### Run Tests

```bash
make test
```

### Clean

```bash
make clean    # remove object files
make fclean   # remove object files and binaries
make re       # full rebuild
```

## Input Format

The simulator reads two text files:

**Network file** — defines stations, events, and rail connections:

```
Node CityA
Node CityB
Event Riot 0.05 48h CityA
Event "Passenger's Discomfort" 0.2 30m CityB
Rail CityA CityB 15.0 250.0
```

Each `Rail` line specifies: `from`, `to`, `distance (km)`, `speed limit (km/h)`.

**Train file** — defines trains:

```
TrainAB1 1.87 1.4 CityA CityB 14h10
```

Fields: `name`, `acceleration`, `braking force`, `departure station`, `arrival station`, `departure time`.

## Author

**André Francisco** ([abaiao-r](https://github.com/abaiao-r)) — 42 Lisboa

## License

This project is licensed under the MIT License — see [LICENSE](LICENSE) for details.