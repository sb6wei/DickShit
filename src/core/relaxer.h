#pragma once

#include <unordered_map>
#include <vector>

#include "distance.h"
#include "graph.h"
#include "scheduler.h"
#include "../analysis/profiler.h"
#include "../analysis/trace.h"

namespace core {

class BatchedRelaxer {
 public:
  void batch_relax(const Graph& graph,
                   Distance& distance,
                   const std::vector<int>& nodes,
                   std::size_t level,
                   int bucket_id,
                   HierarchicalBucketScheduler& scheduler,
                   analysis::Profiler* profiler,
                   analysis::Trace* trace) {
    std::unordered_map<int, double> updates;
    for (int node : nodes) {
      double base = distance.get(node);
      for (const auto& edge : graph.neighbors(node)) {
        if (profiler) {
          profiler->record_edge(edge.id);
          profiler->record_relax_attempt();
        }
        double candidate = base + edge.weight;
        auto iter = updates.find(edge.to);
        if (iter == updates.end() || candidate < iter->second) {
          updates[edge.to] = candidate;
        }
      }
    }

    for (const auto& update : updates) {
      int node = update.first;
      double candidate = update.second;
      double current = distance.get(node);
      if (candidate < current) {
        distance.set(node, candidate);
        if (profiler) {
          profiler->record_relax_success();
        }
        if (trace) {
          trace->record_distance(node, candidate);
          trace->record_relax_event(node, current, candidate, level, bucket_id);
        }
        scheduler.add_node(level, node, candidate);
      }
    }
  }
};

}  // namespace core
