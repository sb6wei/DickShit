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
  core::HierarchicalBucketScheduler scheduler(deltas);
  if (valid_source) {
    scheduler.add_node(0, source, 0.0);
  }

  if (profiler) {
    profiler->reset_edges(graph.edge_count());
  }

  core::BatchedRelaxer relaxer;
  core::RecursiveSolver solver(deltas.size(), options.batch_threshold);

  analysis::Trace* active_trace = options.enable_trace ? trace : nullptr;

  while (scheduler.has_work()) {
    auto frontier_opt = scheduler.next_frontier();
    if (!frontier_opt.has_value()) {
      break;
    }
    const auto& frontier = frontier_opt.value();
    solver.solve(graph, distance, frontier.nodes, frontier.level, frontier.bucket_id, scheduler, relaxer,
                 profiler, active_trace);
  }

  Result result;
  result.distances = distance.values();
  if (profiler) {
    result.stats = analysis::from_profiler(*profiler);
  }
  return result;
}

}  // namespace bmssp
