#!/usr/bin/env python3
import csv
import statistics
import sys
from collections import defaultdict


def main(path):
    rows = []
    with open(path, newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            rows.append(row)

    groups = defaultdict(list)
    for row in rows:
        key = (row['algorithm'], row['graph_type'])
        groups[key].append(float(row['elapsed_ms']))

    print('algorithm,graph_type,mean_ms,median_ms,samples')
    for (algorithm, graph_type), values in sorted(groups.items()):
        mean = statistics.mean(values)
        median = statistics.median(values)
        print(f'{algorithm},{graph_type},{mean:.4f},{median:.4f},{len(values)}')


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print('Usage: analyze.py results/summary.csv')
        sys.exit(1)
    main(sys.argv[1])
