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
  double elapsed_ms = 0.0;
};

inline Stats from_profiler(const Profiler& profiler) {
  Stats stats;
  stats.relax_attempts = profiler.relax_attempts();
  stats.relax_successes = profiler.relax_successes();
  stats.max_frontier_size = profiler.max_frontier_size();
  stats.frontier_count = profiler.frontier_count();
  stats.max_recursion_depth = profiler.max_recursion_depth();
  return stats;
}

}  // namespace analysis
