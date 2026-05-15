#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
BUILD_DIR="$ROOT_DIR/build"
RESULTS_DIR="$ROOT_DIR/results"
SUMMARY="$RESULTS_DIR/summary.csv"

mkdir -p "$BUILD_DIR" "$RESULTS_DIR"

cmake -S "$ROOT_DIR" -B "$BUILD_DIR"
cmake --build "$BUILD_DIR" --config Release

"$BUILD_DIR/experiment_runner" "$SUMMARY"

python3 "$ROOT_DIR/scripts/analyze.py" "$SUMMARY" > "$RESULTS_DIR/analysis.csv"
python3 "$ROOT_DIR/scripts/complexity_fit.py" "$SUMMARY" > "$RESULTS_DIR/complexity.csv"
python3 "$ROOT_DIR/scripts/plot.py" "$SUMMARY" "$RESULTS_DIR"
python3 "$ROOT_DIR/scripts/generate_report.py" "$SUMMARY" "$ROOT_DIR/paper/experiments.tex"

printf "Results written to %s\n" "$RESULTS_DIR"
