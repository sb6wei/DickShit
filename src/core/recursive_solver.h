#pragma once

#include <map>
#include <vector>

#include "distance.h"
#include "frontier.h"
#include "relaxer.h"
#include "scheduler.h"
#include "../analysis/profiler.h"
#include "../analysis/trace.h"

namespace core {

class RecursiveSolver {
 public:
  RecursiveSolver(std::size_t max_levels, std::size_t batch_threshold)
      : max_levels_(max_levels), batch_threshold_(batch_threshold) {}

  void solve(const Graph& graph,
             Distance& distance,
             const std::vector<int>& nodes,
             std::size_t level,
             int bucket_id,
             HierarchicalBucketScheduler& scheduler,
             BatchedRelaxer& relaxer,
             analysis::Profiler* profiler,
             analysis::Trace* trace,
             std::size_t depth = 0) {
    if (profiler) {
      profiler->record_frontier(nodes.size());
      profiler->record_recursion_depth(depth);
    }
    if (nodes.empty()) {
      return;
    }
    if (level + 1 >= max_levels_ || nodes.size() <= batch_threshold_) {
      relaxer.batch_relax(graph, distance, nodes, level, bucket_id, scheduler, profiler, trace);
      return;
    }

    double next_delta = scheduler.delta(level + 1);
    std::map<int, std::vector<int>> groups;
    for (int node : nodes) {
      double value = distance.get(node);
      int next_bucket = static_cast<int>(std::floor(value / next_delta));
      groups[next_bucket].push_back(node);
    }

    for (auto& entry : groups) {
      solve(graph, distance, entry.second, level + 1, entry.first, scheduler, relaxer, profiler, trace,
            depth + 1);
    }
  }

 private:
  std::size_t max_levels_ = 1;
  std::size_t batch_threshold_ = 32;
};

}  // namespace core
