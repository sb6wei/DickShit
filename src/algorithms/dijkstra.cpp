#include "dijkstra.h"

#include <queue>
#include <utility>
#include <vector>

#include "../core/distance.h"

namespace dijkstra {

Result run(const core::Graph& graph,
           int source,
           analysis::Profiler* profiler,
           analysis::Trace* trace) {
  core::Distance distance(graph.node_count());
  bool valid_source = source >= 0 && static_cast<std::size_t>(source) < graph.node_count();
  if (valid_source) {
    distance.set(source, 0.0);
  }
  if (profiler) {
    profiler->reset_edges(graph.edge_count());
  }

  using Entry = std::pair<double, int>;
  std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pq;
  if (!valid_source) {
    Result result;
    result.distances = distance.values();
    if (profiler) {
      result.stats = analysis::from_profiler(*profiler);
    }
    return result;
  }
  pq.push({0.0, source});

  while (!pq.empty()) {
    auto [dist, node] = pq.top();
    pq.pop();
    if (dist != distance.get(node)) {
      continue;
    }
    for (const auto& edge : graph.neighbors(node)) {
      if (profiler) {
        profiler->record_edge(edge.id);
        profiler->record_relax_attempt();
      }
      double candidate = dist + edge.weight;
      if (candidate < distance.get(edge.to)) {
        double current = distance.get(edge.to);
        distance.set(edge.to, candidate);
        if (profiler) {
          profiler->record_relax_success();
        }
        if (trace) {
          trace->record_distance(edge.to, candidate);
          trace->record_relax_event(edge.to, current, candidate, 0, 0);
        }
        pq.push({candidate, edge.to});
      }
    }
  }

  Result result;
  result.distances = distance.values();
  if (profiler) {
    result.stats = analysis::from_profiler(*profiler);
  }
  return result;
}

}  // namespace dijkstra
