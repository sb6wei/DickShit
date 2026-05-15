#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "../core/bmssp.h"
#include "../algorithms/dijkstra.h"
#include "../analysis/export.h"

namespace {

struct BenchmarkArgs {
  std::string type = "random";
  std::string input_path;
  std::string output_path = "results/summary.csv";
  std::string trace_prefix = "results/trace";
  std::size_t nodes = 1000;
  std::size_t edges = 5000;
  std::size_t rows = 0;
  std::size_t cols = 0;
  int source = 0;
  unsigned int seed = 42;
  bool enable_trace = false;
  bool export_edges = false;
};

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
  graph.finalize();
  return graph;
}

core::Graph make_sparse_graph(std::size_t n, unsigned int seed) {
  std::size_t m = std::max<std::size_t>(n * 2, 1);
  return make_random_graph(n, m, seed);
}

core::Graph make_grid_graph(std::size_t rows, std::size_t cols) {
  std::size_t n = rows * cols;
  core::Graph graph(n);
  auto id = [cols](std::size_t r, std::size_t c) { return static_cast<int>(r * cols + c); };
  for (std::size_t r = 0; r < rows; ++r) {
    for (std::size_t c = 0; c < cols; ++c) {
      if (c + 1 < cols) {
        graph.add_edge(id(r, c), id(r, c + 1), 1.0);
      }
      if (r + 1 < rows) {
        graph.add_edge(id(r, c), id(r + 1, c), 1.0);
      }
    }
  }
  graph.finalize();
  return graph;
}

BenchmarkArgs parse_args(int argc, char** argv) {
  BenchmarkArgs args;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    auto next_value = [&]() -> std::string {
      if (i + 1 < argc) {
        return argv[++i];
      }
      return "";
    };
    if (arg == "--type") {
      args.type = next_value();
    } else if (arg == "--nodes") {
      args.nodes = static_cast<std::size_t>(std::stoul(next_value()));
    } else if (arg == "--edges") {
      args.edges = static_cast<std::size_t>(std::stoul(next_value()));
    } else if (arg == "--rows") {
      args.rows = static_cast<std::size_t>(std::stoul(next_value()));
    } else if (arg == "--cols") {
      args.cols = static_cast<std::size_t>(std::stoul(next_value()));
    } else if (arg == "--source") {
      args.source = std::stoi(next_value());
    } else if (arg == "--seed") {
      args.seed = static_cast<unsigned int>(std::stoul(next_value()));
    } else if (arg == "--output") {
      args.output_path = next_value();
    } else if (arg == "--input") {
      args.input_path = next_value();
    } else if (arg == "--trace") {
      args.enable_trace = true;
    } else if (arg == "--trace-prefix") {
      args.trace_prefix = next_value();
    } else if (arg == "--export-edges") {
      args.export_edges = true;
    }
  }
  return args;
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

}  // namespace

int main(int argc, char** argv) {
  BenchmarkArgs args = parse_args(argc, argv);

  core::Graph graph;
  std::string graph_type = args.type;
  if (!args.input_path.empty()) {
    graph = core::Graph::load_from_file(args.input_path);
    graph_type = "file";
  } else if (args.type == "random") {
    graph = make_random_graph(args.nodes, args.edges, args.seed);
  } else if (args.type == "sparse") {
    graph = make_sparse_graph(args.nodes, args.seed);
  } else if (args.type == "grid") {
    std::size_t rows = args.rows;
    std::size_t cols = args.cols;
    if (rows == 0 || cols == 0) {
      std::size_t side = static_cast<std::size_t>(std::sqrt(static_cast<double>(args.nodes)));
      rows = side;
      cols = side;
    }
    graph = make_grid_graph(rows, cols);
  } else {
    std::cerr << "Unknown graph type: " << args.type << '\n';
    return 1;
  }

  bmssp::Options options;
  options.enable_trace = args.enable_trace;

  analysis::Profiler bmssp_profiler(graph.edge_count());
  analysis::Trace bmssp_trace;
  auto start_bmssp = std::chrono::steady_clock::now();
  auto bmssp_result = bmssp::run(graph, args.source, options, &bmssp_profiler, &bmssp_trace);
  auto end_bmssp = std::chrono::steady_clock::now();
  double bmssp_ms = std::chrono::duration<double, std::milli>(end_bmssp - start_bmssp).count();

  analysis::Profiler dijkstra_profiler(graph.edge_count());
  analysis::Trace dijkstra_trace;
  auto start_dijkstra = std::chrono::steady_clock::now();
  auto dijkstra_result = dijkstra::run(graph, args.source, &dijkstra_profiler, &dijkstra_trace);
  auto end_dijkstra = std::chrono::steady_clock::now();
  double dijkstra_ms = std::chrono::duration<double, std::milli>(end_dijkstra - start_dijkstra).count();

  analysis::append_stats_csv(args.output_path,
                             annotate_stats(bmssp_result.stats, "bmssp", graph_type, graph, bmssp_ms));
  analysis::append_stats_csv(
      args.output_path, annotate_stats(dijkstra_result.stats, "dijkstra", graph_type, graph, dijkstra_ms));

  if (args.export_edges) {
    analysis::export_edge_usage(args.trace_prefix + "_bmssp_edges.csv", bmssp_profiler);
    analysis::export_edge_usage(args.trace_prefix + "_dijkstra_edges.csv", dijkstra_profiler);
  }

  if (args.enable_trace) {
    analysis::export_trace(args.trace_prefix + "_bmssp_trace.csv", bmssp_trace);
    analysis::export_trace(args.trace_prefix + "_dijkstra_trace.csv", dijkstra_trace);
  }

  std::cout << "BMSSP ms: " << bmssp_ms << " | Dijkstra ms: " << dijkstra_ms << '\n';
  return 0;
}
