#!/usr/bin/env bash
# ─────────────────────────────────────────────────────────────────────────────
# showcase_modes.sh — Run the three showcase networks in all applicable modes
#                     to visualise the differences between Distance, Time and
#                     Congestion routing.
#
# Usage:  ./scripts/showcase_modes.sh            (default: summary only)
#         ./scripts/showcase_modes.sh --full      (full simulation output)
# ─────────────────────────────────────────────────────────────────────────────
set -euo pipefail

BIN="bin/train_yourself"
NET="input/railNetworkPrintFolder"
TRN="input/trainPrintFolder"
FULL=false
[[ "${1:-}" == "--full" ]] && FULL=true

BOLD="\033[1m"
CYAN="\033[36m"
GREEN="\033[32m"
YELLOW="\033[33m"
RESET="\033[0m"

header() { printf "\n${BOLD}${CYAN}══════ %s ══════${RESET}\n" "$1"; }
mode()   { printf "${BOLD}${YELLOW}── %s ──${RESET}\n" "$1"; }

run_mode() {
    local label=$1; shift
    mode "$label"
    if $FULL; then
        "$@" 2>&1
    else
        "$@" 2>&1 | grep -E "path:|REROUTE|Arrived"
    fi
    echo
}

# ─── Showcase 1: Distance vs Time ────────────────────────────────────────────
header "Showcase 1 · Distance vs Time"
echo "Network:  Westville ←→ Eastport via short-slow (80km@60) or long-fast (250km@300)"
echo "Expected: Distance picks Oldbridge (shorter). Time picks Rapidway (faster)."
echo

run_mode "Distance (default)" \
    "$BIN" "$NET/railNetworkShowcaseDistVsTime.txt" "$TRN/trainPrintShowcaseDistVsTime.txt"

run_mode "Time (--time)" \
    "$BIN" "$NET/railNetworkShowcaseDistVsTime.txt" "$TRN/trainPrintShowcaseDistVsTime.txt" --time

# ─── Showcase 2: Congestion Load-Balancing ───────────────────────────────────
header "Showcase 2 · Congestion Load-Balancing"
echo "Network:  Origin → Fork → {MainA→MainB | AltA→AltB} → Dest"
echo "Expected: Distance/Time = all trains on MainA corridor."
echo "          Congestion     = trains alternate between MainA and AltA."
echo

run_mode "Distance (default)" \
    "$BIN" "$NET/railNetworkShowcaseCongestion.txt" "$TRN/trainPrintShowcaseCongestion.txt"

run_mode "Congestion (--congestion)" \
    "$BIN" "$NET/railNetworkShowcaseCongestion.txt" "$TRN/trainPrintShowcaseCongestion.txt" --congestion

# ─── Showcase 3: All Three Modes ─────────────────────────────────────────────
header "Showcase 3 · All Three Modes"
echo "Network:  Capital → {Suburbs(slow) | Express(fast)} → Hub → {MainLine | Bypass} → Terminal"
echo "Expected: Distance  = Suburbs path, all on MainLine."
echo "          Time      = Express path, all on MainLine."
echo "          Congestion = Express path, some trains reroute to Bypass."
echo

run_mode "Distance (default)" \
    "$BIN" "$NET/railNetworkShowcaseAllModes.txt" "$TRN/trainPrintShowcaseAllModes.txt"

run_mode "Time (--time)" \
    "$BIN" "$NET/railNetworkShowcaseAllModes.txt" "$TRN/trainPrintShowcaseAllModes.txt" --time

run_mode "Congestion (--congestion)" \
    "$BIN" "$NET/railNetworkShowcaseAllModes.txt" "$TRN/trainPrintShowcaseAllModes.txt" --congestion

printf "${BOLD}${GREEN}Done.${RESET}\n"
