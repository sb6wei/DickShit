#include "../core/bmssp.h"

#include <cmath>

#include "../core/recursive_solver.h"
#include "../core/relaxer.h"

namespace bmssp {

Result run(const core::Graph& graph,
           int source,
           const Options& options,
           analysis::Profiler* profiler,
           analysis::Trace* trace) {
  core::Distance distance(graph.node_count());
  bool valid_source = source >= 0 && static_cast<std::size_t>(source) < graph.node_count();
  if (valid_source) {
    distance.set(source, 0.0);
  }

  double max_edge_weight = graph.max_edge_weight();
  if (max_edge_weight <= 0.0) {
    max_edge_weight = 1.0;
  }
  double delta0 = options.delta0;
  if (delta0 <= 0.0) {
    delta0 = max_edge_weight * std::max(1.0, std::pow(static_cast<double>(graph.node_count()), 1.0 / 3.0));
  }

  auto deltas = core::build_deltas(delta0, options.max_levels, options.min_delta);
  if (deltas.empty()) {
    deltas.push_back(delta0);
  }
  core::HierarchicalBucketScheduler scheduler(deltas, options.bucket_window, graph.node_count());
  if (valid_source) {
    scheduler.add_node(0, source, 0.0);
  }

  if (profiler) {
    profiler->reset_edges(graph.edge_count());
    profiler->reset_nodes(graph.node_count());
  }

  core::DeltaConfig delta_config;
  delta_config.min_delta = options.min_delta;
  delta_config.adaptive_shrink = options.adaptive_shrink;
  delta_config.depth_shrink = options.depth_shrink;
  delta_config.radius_shrink = options.radius_shrink;
  delta_config.level_radius_scale = options.level_radius_scale;
  delta_config.enable_adaptive = options.enable_approximate;
  core::DeltaController delta_controller(deltas, delta_config);

  core::RelaxerConfig relaxer_config;
  relaxer_config.enable_batching = options.enable_batching;
  relaxer_config.enable_approximate = options.enable_approximate;
  relaxer_config.propagation_radius = options.propagation_radius;
  relaxer_config.max_propagation_radius = options.max_propagation_radius;
  relaxer_config.edge_reuse_cap = options.edge_reuse_cap;

  core::BatchedRelaxer relaxer(graph.node_count(), graph.edge_count());
  relaxer.configure(relaxer_config);

  core::SolverConfig solver_config;
  solver_config.batch_threshold = options.batch_threshold;
  solver_config.enable_recursion = options.enable_recursion;
  solver_config.enable_bucket_grouping = options.enable_bucket_grouping;
  core::RecursiveSolver solver(deltas.size(), solver_config);

  analysis::Trace* active_trace = options.enable_trace ? trace : nullptr;

  while (scheduler.has_work()) {
    auto frontier_opt = scheduler.next_frontier();
    if (!frontier_opt.has_value()) {
      break;
    }
    const auto& frontier = frontier_opt.value();
    solver.solve(graph,
                 distance,
                 frontier.nodes,
                 frontier.level,
                 frontier.bucket_id,
                 delta_controller,
                 scheduler,
                 relaxer,
                 profiler,
                 active_trace);
  }

  Result result;
  result.distances = distance.values();
  if (profiler) {
    result.stats = analysis::from_profiler(*profiler);
  }
  result.stats.memory_bytes =
      graph.memory_bytes() + scheduler.memory_bytes() + relaxer.memory_bytes();
  return result;
}

}  // namespace bmssp
