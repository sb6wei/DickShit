#pragma once

#include <fstream>
#include <string>

#include "stats.h"
#include "trace.h"

namespace analysis {

inline void append_stats_csv(const std::string& path, const Stats& stats) {
  bool file_exists = static_cast<bool>(std::ifstream(path));
  std::ofstream out(path, std::ios::app);
  if (!file_exists) {
    out << "algorithm,graph_type,nodes,edges,relax_attempts,relax_successes,max_frontier_size,frontier_count,max_recursion_depth,elapsed_ms\n";
  }
  out << stats.algorithm << ',' << stats.graph_type << ',' << stats.nodes << ',' << stats.edges << ','
      << stats.relax_attempts << ',' << stats.relax_successes << ',' << stats.max_frontier_size << ','
      << stats.frontier_count << ',' << stats.max_recursion_depth << ',' << stats.elapsed_ms << '\n';
}

inline void export_edge_usage(const std::string& path, const Profiler& profiler) {
  std::ofstream out(path);
  out << "edge_id,usage\n";
  const auto& usage = profiler.edge_usage();
  for (std::size_t i = 0; i < usage.size(); ++i) {
    out << i << ',' << usage[i] << '\n';
  }
}

inline void export_trace(const std::string& path, const Trace& trace) {
  std::ofstream out(path);
  out << "event,node,old_distance,new_distance,level,bucket_id\n";
  for (const auto& event : trace.relax_events()) {
    out << "relax," << event.node << ',' << event.old_distance << ',' << event.new_distance << ','
        << event.level << ',' << event.bucket_id << '\n';
  }
  for (const auto& event : trace.distance_events()) {
    out << "distance," << event.node << ',' << event.distance << ",,,\n";
  }
}

}  // namespace analysis
