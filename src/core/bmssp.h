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
