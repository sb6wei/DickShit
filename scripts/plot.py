#!/usr/bin/env python3
"""Generate performance plots from benchmark results."""
import csv
import sys
from collections import defaultdict
from pathlib import Path

try:
    import matplotlib.pyplot as plt
except ImportError:  # pragma: no cover
    print('matplotlib is required for plotting')
    sys.exit(1)


def read_benchmark_data(path):
    """Read and parse CSV benchmark data."""
    grouped = defaultdict(list)
    try:
        with open(path, newline='') as csvfile:
            reader = csv.DictReader(csvfile)
            for row in reader:
                key = (row['graph_type'], row['algorithm'])
                grouped[key].append((int(row['nodes']), float(row['elapsed_ms'])))
    except (FileNotFoundError, KeyError, ValueError) as e:
        print(f'Error reading {path}: {e}', file=sys.stderr)
        sys.exit(1)
    return grouped


def main(path, output_dir):
    """Generate and save plots for each graph type."""
    grouped = read_benchmark_data(path)
    
    # Pre-compute graph types and algorithms mapping
    graph_types_algorithms = defaultdict(set)
    for (graph_type, algorithm) in grouped.keys():
        graph_types_algorithms[graph_type].add(algorithm)
    
    # Create output directory if needed
    Path(output_dir).mkdir(parents=True, exist_ok=True)
    
    for graph_type in sorted(graph_types_algorithms.keys()):
        plt.figure(figsize=(10, 6))
        
        for algorithm in sorted(graph_types_algorithms[graph_type]):
            points = sorted(grouped[(graph_type, algorithm)])
            xs, ys = zip(*points)  # Efficient unpacking
            plt.plot(xs, ys, marker='o', label=algorithm, linewidth=2)
        
        plt.xlabel('Nodes', fontsize=12)
        plt.ylabel('Elapsed (ms)', fontsize=12)
        plt.title(f'BMSSP vs Dijkstra ({graph_type})', fontsize=14)
        plt.legend()
        plt.grid(True, alpha=0.3)
        plt.tight_layout()
        plt.savefig(f'{output_dir}/{graph_type}_runtime.png', dpi=150)
        plt.close()


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: plot.py results/summary.csv results')
        sys.exit(1)
    main(sys.argv[1], sys.argv[2])
