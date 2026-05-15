#!/usr/bin/env python3
import csv
import sys
from collections import defaultdict

try:
    import matplotlib.pyplot as plt
except ImportError:  # pragma: no cover
    print('matplotlib is required for plotting')
    sys.exit(1)


def main(path, output_dir):
    grouped = defaultdict(list)
    with open(path, newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            key = (row['graph_type'], row['algorithm'])
            grouped[key].append((int(row['nodes']), float(row['elapsed_ms'])))

    graph_types = sorted({key[0] for key in grouped})
    for graph_type in graph_types:
        plt.figure()
        for algorithm in sorted({k[1] for k in grouped if k[0] == graph_type}):
            points = sorted(grouped[(graph_type, algorithm)])
            xs = [p[0] for p in points]
            ys = [p[1] for p in points]
            plt.plot(xs, ys, marker='o', label=algorithm)
        plt.xlabel('Nodes')
        plt.ylabel('Elapsed (ms)')
        plt.title(f'BMSSP vs Dijkstra ({graph_type})')
        plt.legend()
        plt.tight_layout()
        plt.savefig(f'{output_dir}/{graph_type}_runtime.png')
        plt.close()


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print('Usage: plot.py results/summary.csv results')
        sys.exit(1)
    main(sys.argv[1], sys.argv[2])
