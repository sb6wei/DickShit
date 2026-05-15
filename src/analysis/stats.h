#pragma once

#include <cstddef>
#include <string>

#include "profiler.h"

namespace analysis {

struct Stats {
  std::string algorithm;
  std::string graph_type;
  std::size_t nodes = 0;
  std::size_t edges = 0;
  std::size_t relax_attempts = 0;
  std::size_t relax_successes = 0;
  std::size_t max_frontier_size = 0;
  std::size_t frontier_count = 0;
  std::size_t max_recursion_depth = 0;
  double avg_edge_usage = 0.0;
  std::size_t max_edge_usage = 0;
  double avg_frontier_size = 0.0;
  double avg_frontier_overlap = 0.0;
  double max_frontier_overlap = 0.0;
  double avg_recursion_depth = 0.0;
  std::size_t memory_bytes = 0;
  double elapsed_ms = 0.0;
};

inline Stats from_profiler(const Profiler& profiler) {
  Stats stats;
  stats.relax_attempts = profiler.relax_attempts();
  stats.relax_successes = profiler.relax_successes();
  stats.max_frontier_size = profiler.max_frontier_size();
  stats.frontier_count = profiler.frontier_count();
  stats.max_recursion_depth = profiler.max_recursion_depth();
  const auto& usage = profiler.edge_usage();
  if (!usage.empty()) {
    std::size_t sum = 0;
    std::size_t max_usage = 0;
    for (std::size_t value : usage) {
      sum += value;
      max_usage = std::max(max_usage, value);
    }
    stats.avg_edge_usage = static_cast<double>(sum) / static_cast<double>(usage.size());
    stats.max_edge_usage = max_usage;
  }
  const auto& frontier_sizes = profiler.frontier_sizes();
  if (!frontier_sizes.empty()) {
    std::size_t sum = 0;
    for (std::size_t value : frontier_sizes) {
      sum += value;
    }
    stats.avg_frontier_size = static_cast<double>(sum) / static_cast<double>(frontier_sizes.size());
  }
  const auto& overlaps = profiler.frontier_overlaps();
  if (!overlaps.empty() && overlaps.size() == frontier_sizes.size()) {
    double sum_ratio = 0.0;
    double max_ratio = 0.0;
    for (std::size_t i = 0; i < overlaps.size(); ++i) {
      if (frontier_sizes[i] == 0) {
        continue;
      }
      double ratio = static_cast<double>(overlaps[i]) / static_cast<double>(frontier_sizes[i]);
      sum_ratio += ratio;
      max_ratio = std::max(max_ratio, ratio);
    }
    stats.avg_frontier_overlap = sum_ratio / static_cast<double>(overlaps.size());
    stats.max_frontier_overlap = max_ratio;
  }
  const auto& depths = profiler.recursion_depths();
  if (!depths.empty()) {
    std::size_t sum = 0;
    for (std::size_t value : depths) {
      sum += value;
    }
    stats.avg_recursion_depth = static_cast<double>(sum) / static_cast<double>(depths.size());
  }
  return stats;
}

}  // namespace analysis
