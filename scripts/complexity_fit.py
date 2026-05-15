#!/usr/bin/env python3
import csv
import math
import sys
from collections import defaultdict


def fit_loglog(points):
    xs = [math.log(p[0]) for p in points]
    ys = [math.log(max(p[1], 1e-9)) for p in points]
    n = len(points)
    if n < 2:
        return float('nan')
    mean_x = sum(xs) / n
    mean_y = sum(ys) / n
    num = sum((x - mean_x) * (y - mean_y) for x, y in zip(xs, ys))
    den = sum((x - mean_x) ** 2 for x in xs)
    if den == 0:
        return float('nan')
    return num / den


def main(path):
    rows = []
    with open(path, newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            rows.append(row)

    grouped = defaultdict(list)
    for row in rows:
        key = (row['algorithm'], row['graph_type'])
        grouped[key].append((int(row['nodes']), float(row['elapsed_ms'])))

    print('algorithm,graph_type,empirical_exponent')
    for key, points in sorted(grouped.items()):
        points = sorted(points)
        exponent = fit_loglog(points)
        algorithm, graph_type = key
        print(f'{algorithm},{graph_type},{exponent:.4f}')


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: complexity_fit.py results/summary.csv')
        sys.exit(1)
    main(sys.argv[1])
