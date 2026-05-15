# BMSSP-Style Directed SSSP Framework

This repository provides a research-oriented reproduction framework for a BMSSP-style hierarchical batching approach to directed single-source shortest paths. The framework emphasizes recursive bucket decomposition, approximate distance grouping, batched relaxation, and empirical complexity analysis, alongside a Dijkstra baseline.

## Project Structure

```
project/
├── src/
│   ├── core/
│   ├── algorithms/
│   ├── analysis/
│   ├── experiments/
├── scripts/
├── paper/
├── results/
└── CMakeLists.txt
```

## Build

```bash
cmake -S . -B build
cmake --build build --config Release
```

## Benchmark Input Format

Graph files are plain text:

```
n m
u0 v0 w0
u1 v1 w1
...
```

- `n`: number of nodes
- `m`: number of directed edges
- each edge uses zero-based indices and a positive weight

## Run Benchmark (single case)

```bash
./build/benchmark --type random --nodes 1000 --edges 5000 --output results/summary.csv
./build/benchmark --type sparse --nodes 1000
./build/benchmark --type grid --rows 64 --cols 64
./build/benchmark --input data/graph.txt
```

Optional flags:
- `--trace` to export trace logs
- `--trace-prefix results/trace` for trace outputs
- `--export-edges` to export edge participation counts

## Run Full Experiments

```bash
bash scripts/run_experiments.sh
```

This script builds the project, runs the experiment runner (including an ablation with recursion disabled), performs complexity fitting, generates plots, and regenerates the LaTeX experiments section.

## Outputs

- `results/summary.csv`: per-run statistics
- `results/analysis.csv`: aggregated mean/median stats
- `results/complexity.csv`: empirical exponent estimation
- `results/*_runtime.png`: matplotlib plots
- `paper/experiments.tex`: auto-generated experiment table

## Python Dependencies

The analysis and plotting scripts require Python 3. For plotting, install `matplotlib` (and optionally `numpy` for your own analysis).

## Notes

- The BMSSP-style implementation avoids a global priority queue and instead uses hierarchical bucket scheduling.
- The Dijkstra baseline uses `std::priority_queue` for comparison only.
