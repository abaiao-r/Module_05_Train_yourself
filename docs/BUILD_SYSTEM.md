# Build System & Testing

> Back to [TECHNICAL.md](TECHNICAL.md) · [README](../README.md)

---

## Build System

Plain `Makefile` with:
- **No relink**: pattern rules produce `.o` in `objs/`, binary only re-links if objects change
- **Compiler**: `CXX = c++` (overridable: `make CXX=g++`)
- **Flags**: `-Wall -Wextra -Werror -std=c++17 -g`
- **Dependency check**: `make setup` auto-detects platform and installs missing tools (compiler + Qt)
- **qmake auto-detect**: Makefile finds qmake6/qmake across macOS Homebrew and Linux system paths

---

## Targets

| Target | Description |
|--------|-------------|
| `make` | Build the CLI simulator |
| `make test` | Run all 498 tests |
| `make run` | Run with sample data |
| `make run-time` | Run with time-based routing |
| `make run-animate` | Run with terminal animation |
| `make run-multi` | Run 1000 simulations with statistics |
| `make run-graph` | Export Graphviz DOT + render |
| `make run-graph-time` | Export graph with time-based routing |
| `make bonus` | Build the Qt GUI (checks Qt 6 first) |
| `make run-gui` | Build & launch the GUI |
| `make setup` | Check & install all dependencies (compiler + Qt) |
| `make clean` | Remove object files |
| `make fclean` | Remove objects + binaries |
| `make bonus-clean` | Remove GUI build artifacts |
| `make re` | Full rebuild |

---

## Dependency Management

The `scripts/check_deps.sh` script auto-detects the platform and manages dependencies:

| Platform | Package Manager | Detection |
|----------|----------------|-----------|
| macOS | Homebrew | `uname -s` |
| Ubuntu / Debian | apt | `/etc/os-release` |
| Fedora | dnf | `/etc/os-release` |
| Arch | pacman | `/etc/os-release` |
| WSL | (same as above) | `uname -r` contains `microsoft` |

**Usage:**
```bash
make setup      # check & install all dependencies (compiler, make, Qt >= 6.3)
```

---

## Testing

**498 tests** across **13 suites**, all using the custom `TestFramework.hpp`:

| Suite | Tests | Coverage |
|-------|------:|----------|
| NodeTest | 5 | Construction, naming, exceptions |
| RailNetworkTest | 14 | Graph operations, validation |
| TrainTest | 16 | Properties, physics calculations |
| EventTest | 13 | Probability, duration, rail events |
| DijkstraTest | 11 | Shortest path, weight modes |
| InputHandlerTest | 28 | Parsing, validation, error handling |
| TrainFactoryTest | 13 | Factory method, ID assignment |
| OutputManagerTest | 5 | Formatting, file output |
| IntegrationTest | 7 | End-to-end with .result file checks |
| EdgeCaseTest | 9 | Boundary values, error paths |
| CombinationTest | 315 | Exhaustive parameter combinations |
| EndToEndTest | 26 | Subprocess binary validation, exit codes |
| InputComboTest | 36 | All input file pairs, ASan leak checks |
| **Total** | **498** | |

### Test Framework

Custom `TestFramework.hpp` provides:
- `ASSERT_TRUE`, `ASSERT_FALSE`, `ASSERT_EQ`, `ASSERT_STR_EQ`, `ASSERT_THROWS`
- Coloured terminal output (green pass / red fail)
- Each test binary is a standalone executable returning 0 (all pass) or 1 (any failure)
- Compatible with CI and valgrind

### Running Tests

```bash
make test     # run all 498 tests
```

---

## CI Pipeline

GitHub Actions on every pull request to `master`/`main`:
- **Matrix**: GCC + Clang
- **Steps**: build → `make test` → valgrind on every binary (leak-check=full, error-exitcode=1)
