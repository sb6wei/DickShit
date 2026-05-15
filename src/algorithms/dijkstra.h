#pragma once

#include <vector>

#include "../analysis/profiler.h"
#include "../analysis/trace.h"
#include "../analysis/stats.h"
#include "../core/graph.h"

namespace dijkstra {

struct Result {
  std::vector<double> distances;
  analysis::Stats stats;
};

Result run(const core::Graph& graph,
           int source,
           analysis::Profiler* profiler,
           analysis::Trace* trace);

}  // namespace dijkstra
