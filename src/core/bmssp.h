#pragma once

#include <cstddef>
#include <vector>

#include "distance.h"
#include "graph.h"
#include "scheduler.h"
#include "../analysis/profiler.h"
#include "../analysis/stats.h"
#include "../analysis/trace.h"

namespace bmssp {

struct Options {
  double delta0 = 0.0;
  double min_delta = 1e-3;
  std::size_t max_levels = 8;
  std::size_t batch_threshold = 64;
  bool enable_trace = false;
  bool enable_recursion = true;
  bool enable_batching = true;
  bool enable_bucket_grouping = true;
  bool enable_approximate = true;
  std::size_t bucket_window = 256;
  int propagation_radius = 1;
  int max_propagation_radius = 3;
  std::size_t edge_reuse_cap = 8;
  double adaptive_shrink = 0.85;
  double depth_shrink = 0.12;
  double radius_shrink = 0.6;
  double level_radius_scale = 0.25;
};

struct Result {
  std::vector<double> distances;
  analysis::Stats stats;
};

Result run(const core::Graph& graph,
           int source,
           const Options& options,
           analysis::Profiler* profiler,
           analysis::Trace* trace);

}  // namespace bmssp
