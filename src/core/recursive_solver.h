#pragma once

#include <algorithm>
#include <map>
#include <memory_resource>
#include <vector>

#include "distance.h"
#include "frontier.h"
#include "relaxer.h"
#include "scheduler.h"
#include "../analysis/profiler.h"
#include "../analysis/trace.h"

namespace core {

struct SolverConfig {
  std::size_t batch_threshold = 32;
  bool enable_recursion = true;
  bool enable_bucket_grouping = true;
};

class RecursiveSolver {
 public:
  explicit RecursiveSolver(std::size_t max_levels, SolverConfig config)
      : max_levels_(max_levels), config_(config) {}

  template <typename NodeRange>
  void solve(const Graph& graph,
             Distance& distance,
             const NodeRange& nodes,
             std::size_t level,
             int bucket_id,
             const DeltaController& delta_controller,
             HierarchicalBucketScheduler& scheduler,
             BatchedRelaxer& relaxer,
             analysis::Profiler* profiler,
             analysis::Trace* trace,
             std::size_t depth = 0,
             std::size_t parent_event = 0) {
    if (nodes.empty()) {
      return;
    }

    double min_value = Distance::infinity();
    double max_value = 0.0;
    for (int node : nodes) {
      double value = distance.get(node);
      min_value = std::min(min_value, value);
      max_value = std::max(max_value, value);
    }
    double radius = max_value >= min_value ? (max_value - min_value) : 0.0;

    if (profiler) {
      profiler->record_frontier(nodes, depth);
    }

    std::size_t current_event = 0;
    if (trace) {
      current_event = trace->record_recursion_event(parent_event,
                                                    depth,
                                                    level,
                                                    bucket_id,
                                                    static_cast<std::size_t>(nodes.size()),
                                                    radius);
    }

    if (!config_.enable_recursion || level + 1 >= max_levels_ ||
        nodes.size() <= config_.batch_threshold) {
      relaxer.batch_relax(graph,
                          distance,
                          nodes,
                          level,
                          bucket_id,
                          radius,
                          depth,
                          scheduler,
                          profiler,
                          trace);
      return;
    }

    double next_delta = delta_controller.effective_delta(level + 1, depth + 1, radius);
    if (profiler) {
      profiler->record_delta(level + 1, depth + 1, next_delta, radius);
    }
    if (trace) {
      trace->record_delta_event(level + 1, depth + 1, delta_controller.base_delta(level + 1), next_delta, radius);
    }

    if (!config_.enable_bucket_grouping) {
      int next_bucket = static_cast<int>(std::floor(min_value / next_delta));
      solve(graph,
            distance,
            nodes,
            level + 1,
            next_bucket,
            delta_controller,
            scheduler,
            relaxer,
            profiler,
            trace,
            depth + 1,
            current_event);
      return;
    }

    std::map<int, NodeList> groups;
    auto* resource = std::pmr::get_default_resource();
    for (int node : nodes) {
      double value = distance.get(node);
      int next_bucket = static_cast<int>(std::floor(value / next_delta));
      auto iter = groups.find(next_bucket);
      if (iter == groups.end()) {
        iter = groups.emplace(next_bucket, NodeList(resource)).first;
      }
      iter->second.push_back(node);
    }

    for (auto& entry : groups) {
      solve(graph,
            distance,
            entry.second,
            level + 1,
            entry.first,
            delta_controller,
            scheduler,
            relaxer,
            profiler,
            trace,
            depth + 1,
            current_event);
    }
  }

 private:
  std::size_t max_levels_ = 1;
  SolverConfig config_;
};

}  // namespace core
