#!/usr/bin/env bash
# ============================================================================ #
#  check_deps.sh — Detect, verify, and (optionally) install build dependencies #
#                                                                               #
#  Usage:                                                                       #
#    ./scripts/check_deps.sh cli          # check CLI build deps               #
#    ./scripts/check_deps.sh gui          # check CLI + Qt GUI deps            #
#    ./scripts/check_deps.sh install-cli  # install CLI deps                   #
#    ./scripts/check_deps.sh install-gui  # install CLI + Qt deps              #
# ============================================================================ #
set -euo pipefail

# ── Colours ──────────────────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'
CYAN='\033[0;36m'; BOLD='\033[1m'; NC='\033[0m'

ok()   { printf "  ${GREEN}✓${NC} %s\n" "$1"; }
fail() { printf "  ${RED}✗${NC} %s\n" "$1"; }
warn() { printf "  ${YELLOW}!${NC} %s\n" "$1"; }
info() { printf "  ${CYAN}→${NC} %s\n" "$1"; }

ERRORS=0
MODE="${1:-cli}"

# ── Detect platform ─────────────────────────────────────────────────────────
detect_os() {
    if [[ "$(uname)" == "Darwin" ]]; then
        echo "macos"
    elif grep -qi microsoft /proc/version 2>/dev/null; then
        echo "wsl"
    elif [[ "$(uname)" == "Linux" ]]; then
        echo "linux"
    else
        echo "unknown"
    fi
}

detect_distro() {
    if command -v apt-get &>/dev/null; then
        echo "debian"
    elif command -v dnf &>/dev/null; then
        echo "fedora"
    elif command -v pacman &>/dev/null; then
        echo "arch"
    else
        echo "unknown"
    fi
}

OS="$(detect_os)"
DISTRO=""
[[ "$OS" == "linux" || "$OS" == "wsl" ]] && DISTRO="$(detect_distro)"

echo ""
printf "${BOLD}══════════════════════════════════════════════════${NC}\n"
printf "${BOLD}  Train Simulation — Dependency Check${NC}\n"
printf "${BOLD}══════════════════════════════════════════════════${NC}\n"
info "Platform : $OS$([ -n "$DISTRO" ] && echo " ($DISTRO)" || true)"
info "Mode     : $MODE"
echo ""

# ── Helper: install command per platform ─────────────────────────────────────
install_hint_compiler() {
    case "$OS" in
        macos) echo "  Run: ${CYAN}xcode-select --install${NC}" ;;
        *)
            case "$DISTRO" in
                debian) echo "  Run: ${CYAN}sudo apt-get update && sudo apt-get install -y build-essential${NC}" ;;
                fedora) echo "  Run: ${CYAN}sudo dnf install -y gcc-c++ make${NC}" ;;
                arch)   echo "  Run: ${CYAN}sudo pacman -S --noconfirm base-devel${NC}" ;;
                *)      echo "  Install a C++17-capable compiler (g++ >= 7 or clang++ >= 5) and make" ;;
            esac ;;
    esac
}

install_hint_qt() {
    case "$OS" in
        macos) echo "  Run: ${CYAN}brew install qt${NC}" ;;
        *)
            case "$DISTRO" in
                debian) echo "  Run: ${CYAN}sudo apt-get update && sudo apt-get install -y qt6-base-dev${NC}" ;;
                fedora) echo "  Run: ${CYAN}sudo dnf install -y qt6-qtbase-devel${NC}" ;;
                arch)   echo "  Run: ${CYAN}sudo pacman -S --noconfirm qt6-base${NC}" ;;
                *)      echo "  Install Qt 6 base development package for your distribution" ;;
            esac ;;
    esac
}

install_hint_graphviz() {
    case "$OS" in
        macos) echo "  Run: ${CYAN}brew install graphviz${NC}" ;;
        *)
            case "$DISTRO" in
                debian) echo "  Run: ${CYAN}sudo apt-get update && sudo apt-get install -y graphviz${NC}" ;;
                fedora) echo "  Run: ${CYAN}sudo dnf install -y graphviz${NC}" ;;
                arch)   echo "  Run: ${CYAN}sudo pacman -S --noconfirm graphviz${NC}" ;;
                *)      echo "  Install graphviz for your distribution" ;;
            esac ;;
    esac
}

install_hint_brew() {
    echo "  Run: ${CYAN}/bin/bash -c \"\$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)\"${NC}"
}

# ── Auto-install helpers ─────────────────────────────────────────────────────
try_install_compiler() {
    info "Attempting to install C++ compiler..."
    case "$OS" in
        macos)
            if ! xcode-select -p &>/dev/null; then
                warn "Xcode Command Line Tools required. A system dialog may appear."
                xcode-select --install 2>/dev/null || true
                printf "\n${YELLOW}  After the installation dialog completes, re-run: make setup${NC}\n\n"
                exit 1
            fi ;;
        *)
            case "$DISTRO" in
                debian)
                    if command -v sudo &>/dev/null; then
                        sudo apt-get update -qq && sudo apt-get install -y build-essential
                    else
                        fail "sudo not available."
                        echo -e "$(install_hint_compiler)\n"
                        return 1
                    fi ;;
                fedora)
                    if command -v sudo &>/dev/null; then
                        sudo dnf install -y gcc-c++ make
                    else
                        fail "sudo not available."
                        echo -e "$(install_hint_compiler)\n"
                        return 1
                    fi ;;
                arch)
                    if command -v sudo &>/dev/null; then
                        sudo pacman -S --noconfirm base-devel
                    else
                        fail "sudo not available."
                        echo -e "$(install_hint_compiler)\n"
                        return 1
                    fi ;;
                *)
                    fail "Unknown distro — cannot auto-install."
                    echo -e "$(install_hint_compiler)\n"
                    return 1 ;;
            esac ;;
    esac
}

try_install_qt() {
    info "Attempting to install Qt 6..."
    case "$OS" in
        macos)
            if ! command -v brew &>/dev/null; then
                fail "Homebrew is not installed (needed to install Qt on macOS)."
                echo -e "$(install_hint_brew)\n"
                echo -e "$(install_hint_qt)\n"
                return 1
            fi
            if brew list qt &>/dev/null; then
                info "Qt already installed — upgrading to latest..."
                brew upgrade qt || true
            else
                brew install qt
            fi ;;
        *)
            case "$DISTRO" in
                debian)
                    if command -v sudo &>/dev/null; then
                        sudo apt-get update -qq && sudo apt-get install -y qt6-base-dev
                    else
                        fail "sudo not available."
                        echo -e "$(install_hint_qt)\n"
                        return 1
                    fi ;;
                fedora)
                    if command -v sudo &>/dev/null; then
                        sudo dnf install -y qt6-qtbase-devel
                    else
                        fail "sudo not available."
                        echo -e "$(install_hint_qt)\n"
                        return 1
                    fi ;;
                arch)
                    if command -v sudo &>/dev/null; then
                        sudo pacman -S --noconfirm qt6-base
                    else
                        fail "sudo not available."
                        echo -e "$(install_hint_qt)\n"
                        return 1
                    fi ;;
                *)
                    fail "Unknown distro — cannot auto-install."
                    echo -e "$(install_hint_qt)\n"
                    return 1 ;;
            esac ;;
    esac
}

try_install_graphviz() {
    info "Attempting to install Graphviz..."
    case "$OS" in
        macos)
            if ! command -v brew &>/dev/null; then
                fail "Homebrew is not installed (needed to install Graphviz on macOS)."
                echo -e "$(install_hint_brew)\n"
                echo -e "$(install_hint_graphviz)\n"
                return 1
            fi
            brew install graphviz ;;
        *)
            case "$DISTRO" in
                debian)
                    if command -v sudo &>/dev/null; then
                        sudo apt-get update -qq && sudo apt-get install -y graphviz
                    else
                        fail "sudo not available."
                        echo -e "$(install_hint_graphviz)\n"
                        return 1
                    fi ;;
                fedora)
                    if command -v sudo &>/dev/null; then
                        sudo dnf install -y graphviz
                    else
                        fail "sudo not available."
                        echo -e "$(install_hint_graphviz)\n"
                        return 1
                    fi ;;
                arch)
                    if command -v sudo &>/dev/null; then
                        sudo pacman -S --noconfirm graphviz
                    else
                        fail "sudo not available."
                        echo -e "$(install_hint_graphviz)\n"
                        return 1
                    fi ;;
                *)
                    fail "Unknown distro — cannot auto-install."
                    echo -e "$(install_hint_graphviz)\n"
                    return 1 ;;
            esac ;;
    esac
}

# ── Locate qmake (exported for Makefile) ─────────────────────────────────────
find_qmake() {
    # Try common names/paths in order of preference
    for cmd in qmake6 qmake; do
        local p
        p="$(command -v "$cmd" 2>/dev/null || true)"
        if [[ -n "$p" ]]; then echo "$p"; return 0; fi
    done
    # Homebrew paths (macOS Intel + Apple Silicon)
    for p in /opt/homebrew/bin/qmake /usr/local/bin/qmake /opt/homebrew/opt/qt/bin/qmake; do
        if [[ -x "$p" ]]; then echo "$p"; return 0; fi
    done
    return 1
}

# ═══════════════════════════════════════════════════════════════════════════════
#  CHECK / INSTALL — Compiler & Make
# ═══════════════════════════════════════════════════════════════════════════════
check_cli() {
    printf "${BOLD}── C++ Compiler ──${NC}\n"

    if command -v c++ &>/dev/null; then
        # Verify C++17 support
        local tmpf
        tmpf="$(mktemp /tmp/cxx17_test.XXXXXX.cpp)"
        echo 'int main(){}' > "$tmpf"
        if c++ -std=c++17 -x c++ "$tmpf" -o /dev/null 2>/dev/null; then
            ok "c++ supports C++17  —  $(c++ --version 2>&1 | head -1)"
        else
            fail "c++ found but does NOT support C++17"
            echo -e "$(install_hint_compiler)\n"
            ERRORS=$((ERRORS + 1))
        fi
        rm -f "$tmpf" a.out
    else
        fail "No C++ compiler found"
        echo -e "$(install_hint_compiler)\n"
        ERRORS=$((ERRORS + 1))
    fi

    printf "\n${BOLD}── Make ──${NC}\n"
    if command -v make &>/dev/null; then
        ok "make  —  $(make --version 2>&1 | head -1)"
    else
        fail "make not found"
        echo -e "$(install_hint_compiler)\n"
        ERRORS=$((ERRORS + 1))
    fi
}

# ═══════════════════════════════════════════════════════════════════════════════
#  CHECK / INSTALL — Qt 6 + qmake
# ═══════════════════════════════════════════════════════════════════════════════
MIN_QT="6.3"

# Compare two dot-separated version strings: returns 0 if $1 >= $2
version_ge() {
    local IFS=.
    local i a=($1) b=($2)
    for ((i = 0; i < ${#b[@]}; i++)); do
        local va=${a[i]:-0} vb=${b[i]:-0}
        if ((va > vb)); then return 0; fi
        if ((va < vb)); then return 1; fi
    done
    return 0
}

check_gui() {
    printf "\n${BOLD}── Qt 6 (for GUI bonus) — minimum ${MIN_QT} ──${NC}\n"

    local qm
    if qm="$(find_qmake)"; then
        ok "qmake found  —  $qm"
        local ver
        ver="$("$qm" -query QT_VERSION 2>/dev/null || echo "unknown")"
        if [[ "$ver" == 6.* ]]; then
            if version_ge "$ver" "$MIN_QT"; then
                ok "Qt version $ver  (>= $MIN_QT)"
            else
                fail "Qt $ver is too old — minimum required is $MIN_QT"
                echo -e "$(install_hint_qt)\n"
                ERRORS=$((ERRORS + 1))
            fi
        elif [[ "$ver" == 5.* ]]; then
            fail "qmake points to Qt $ver — Qt >= $MIN_QT is required"
            echo -e "$(install_hint_qt)\n"
            ERRORS=$((ERRORS + 1))
        else
            warn "Could not determine Qt version (got: $ver)"
        fi
    else
        fail "qmake not found — Qt >= $MIN_QT is required for the GUI bonus"
        echo -e "$(install_hint_qt)\n"
        ERRORS=$((ERRORS + 1))
    fi
}

# ═══════════════════════════════════════════════════════════════════════════════
#  CHECK / INSTALL — Graphviz (optional — for --graph rendering)
# ═══════════════════════════════════════════════════════════════════════════════
check_graphviz() {
    printf "\n${BOLD}── Graphviz (for --graph PNG/SVG rendering) ──${NC}\n"

    if command -v dot &>/dev/null; then
        ok "dot (graphviz)  —  $(dot -V 2>&1 | head -1)"
    else
        warn "graphviz not found — --graph will produce .dot files but not PNG/SVG"
        echo -e "$(install_hint_graphviz)\n"
        GRAPHVIZ_MISSING=1
    fi
}

# ═══════════════════════════════════════════════════════════════════════════════
#  MAIN
# ═══════════════════════════════════════════════════════════════════════════════
GRAPHVIZ_MISSING=0

case "$MODE" in
    cli)
        check_cli
        check_graphviz
        ;;
    gui)
        check_cli
        check_gui
        check_graphviz
        ;;
    install-cli)
        check_cli
        if [[ $ERRORS -gt 0 ]]; then
            echo ""
            try_install_compiler
            ERRORS=0
            echo ""
            check_cli
        fi
        check_graphviz
        if [[ $GRAPHVIZ_MISSING -gt 0 ]]; then
            echo ""
            try_install_graphviz
            GRAPHVIZ_MISSING=0
            echo ""
            check_graphviz
        fi
        ;;
    install-gui)
        check_cli
        if [[ $ERRORS -gt 0 ]]; then
            echo ""
            try_install_compiler
            ERRORS=0
            echo ""
            check_cli
        fi
        check_gui
        if [[ $ERRORS -gt 0 ]]; then
            echo ""
            try_install_qt
            ERRORS=0
            echo ""
            check_gui
        fi
        check_graphviz
        if [[ $GRAPHVIZ_MISSING -gt 0 ]]; then
            echo ""
            try_install_graphviz
            GRAPHVIZ_MISSING=0
            echo ""
            check_graphviz
        fi
        ;;
    *)
        echo "Usage: $0 {cli|gui|install-cli|install-gui}"
        exit 1
        ;;
esac

# ── Summary ──────────────────────────────────────────────────────────────────
echo ""
if [[ $ERRORS -eq 0 ]]; then
    printf "${GREEN}${BOLD}  All dependencies satisfied ✓${NC}\n\n"
    exit 0
else
    printf "${RED}${BOLD}  $ERRORS missing dependency(ies) — see above for install instructions${NC}\n\n"
    exit 1
fi
