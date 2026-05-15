#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "../algorithms/dijkstra.h"
#include "../analysis/export.h"
#include "../core/bmssp.h"

namespace {

core::Graph make_random_graph(std::size_t n, std::size_t m, unsigned int seed) {
  core::Graph graph(n);
  std::mt19937 rng(seed);
  std::uniform_int_distribution<int> node_dist(0, static_cast<int>(n - 1));
  std::uniform_real_distribution<double> weight_dist(1.0, 10.0);
  for (std::size_t i = 0; i < m; ++i) {
    int u = node_dist(rng);
    int v = node_dist(rng);
    double w = weight_dist(rng);
    graph.add_edge(u, v, w);
  }
  return graph;
}

core::Graph make_sparse_graph(std::size_t n, unsigned int seed) {
  return make_random_graph(n, std::max<std::size_t>(n * 2, 1), seed);
}

core::Graph make_grid_graph(std::size_t side) {
  core::Graph graph(side * side);
  auto id = [side](std::size_t r, std::size_t c) { return static_cast<int>(r * side + c); };
  for (std::size_t r = 0; r < side; ++r) {
    for (std::size_t c = 0; c < side; ++c) {
      if (c + 1 < side) {
        graph.add_edge(id(r, c), id(r, c + 1), 1.0);
      }
      if (r + 1 < side) {
        graph.add_edge(id(r, c), id(r + 1, c), 1.0);
      }
    }
  }
  return graph;
}

analysis::Stats annotate_stats(const analysis::Stats& base,
                               const std::string& algorithm,
                               const std::string& graph_type,
                               const core::Graph& graph,
                               double elapsed_ms) {
  analysis::Stats stats = base;
  stats.algorithm = algorithm;
  stats.graph_type = graph_type;
  stats.nodes = graph.node_count();
  stats.edges = graph.edge_count();
  stats.elapsed_ms = elapsed_ms;
  return stats;
}

void run_bmssp_case(const core::Graph& graph,
                    const std::string& graph_type,
                    int source,
                    const std::string& output_path,
                    const std::string& label,
                    const bmssp::Options& options) {
  analysis::Profiler profiler(graph.edge_count());
  analysis::Trace trace;
  auto start = std::chrono::steady_clock::now();
  auto result = bmssp::run(graph, source, options, &profiler, &trace);
  auto end = std::chrono::steady_clock::now();
  double elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();

  analysis::append_stats_csv(output_path,
                             annotate_stats(result.stats, label, graph_type, graph, elapsed_ms));
}

void run_case(const core::Graph& graph,
              const std::string& graph_type,
              int source,
              const std::string& output_path) {
  bmssp::Options base_options;
  run_bmssp_case(graph, graph_type, source, output_path, "bmssp", base_options);

  bmssp::Options no_recursion = base_options;
  no_recursion.max_levels = 1;
  run_bmssp_case(graph, graph_type, source, output_path, "bmssp_no_recursion", no_recursion);

  analysis::Profiler dijkstra_profiler(graph.edge_count());
  analysis::Trace dijkstra_trace;
  auto start_dijkstra = std::chrono::steady_clock::now();
  auto dijkstra_result = dijkstra::run(graph, source, &dijkstra_profiler, &dijkstra_trace);
  auto end_dijkstra = std::chrono::steady_clock::now();
  double dijkstra_ms = std::chrono::duration<double, std::milli>(end_dijkstra - start_dijkstra).count();

  analysis::append_stats_csv(
      output_path, annotate_stats(dijkstra_result.stats, "dijkstra", graph_type, graph, dijkstra_ms));
}

}  // namespace

int main(int argc, char** argv) {
  std::string output_path = "results/summary.csv";
  int source = 0;
  if (argc > 1) {
    output_path = argv[1];
  }
  if (argc > 2) {
    source = std::stoi(argv[2]);
  }

  std::vector<std::size_t> sizes = {128, 256, 512, 1024, 2048};
  unsigned int seed = 7;
  for (std::size_t n : sizes) {
    run_case(make_random_graph(n, n * 4, seed++), "random", source, output_path);
    run_case(make_sparse_graph(n, seed++), "sparse", source, output_path);
    std::size_t side = static_cast<std::size_t>(std::sqrt(static_cast<double>(n)));
    if (side > 1) {
      run_case(make_grid_graph(side), "grid", source, output_path);
    }
  }

  std::cout << "Experiment runner complete. Results: " << output_path << '\n';
  return 0;
}
