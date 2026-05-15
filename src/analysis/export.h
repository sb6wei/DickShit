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
    out << "algorithm,graph_type,nodes,edges,relax_attempts,relax_successes,max_frontier_size,frontier_count,max_recursion_depth,"
           "avg_edge_usage,max_edge_usage,avg_frontier_size,avg_frontier_overlap,max_frontier_overlap,avg_recursion_depth,"
           "memory_bytes,elapsed_ms\n";
  }
  out << stats.algorithm << ',' << stats.graph_type << ',' << stats.nodes << ',' << stats.edges << ','
      << stats.relax_attempts << ',' << stats.relax_successes << ',' << stats.max_frontier_size << ','
      << stats.frontier_count << ',' << stats.max_recursion_depth << ',' << stats.avg_edge_usage << ','
      << stats.max_edge_usage << ',' << stats.avg_frontier_size << ',' << stats.avg_frontier_overlap << ','
      << stats.max_frontier_overlap << ',' << stats.avg_recursion_depth << ',' << stats.memory_bytes << ','
      << stats.elapsed_ms << '\n';
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

inline void export_frontier_series(const std::string& path, const Profiler& profiler) {
  std::ofstream out(path);
  out << "step,frontier_size,frontier_overlap,recursion_depth\n";
  const auto& sizes = profiler.frontier_sizes();
  const auto& overlaps = profiler.frontier_overlaps();
  const auto& depths = profiler.recursion_depths();
  for (std::size_t i = 0; i < sizes.size(); ++i) {
    std::size_t overlap = i < overlaps.size() ? overlaps[i] : 0;
    std::size_t depth = i < depths.size() ? depths[i] : 0;
    out << i << ',' << sizes[i] << ',' << overlap << ',' << depth << '\n';
  }
}

inline void export_delta_series(const std::string& path, const Profiler& profiler) {
  std::ofstream out(path);
  out << "step,level,depth,delta,radius\n";
  const auto& levels = profiler.delta_levels();
  const auto& depths = profiler.delta_depths();
  const auto& values = profiler.delta_values();
  const auto& radii = profiler.delta_radii();
  std::size_t count = std::min({levels.size(), depths.size(), values.size(), radii.size()});
  for (std::size_t i = 0; i < count; ++i) {
    out << i << ',' << levels[i] << ',' << depths[i] << ',' << values[i] << ',' << radii[i] << '\n';
  }
}

inline void export_recursion_trace(const std::string& path, const Trace& trace) {
  std::ofstream out(path);
  out << "id,parent_id,depth,level,bucket_id,size,radius\n";
  for (const auto& event : trace.recursion_events()) {
    out << event.id << ',' << event.parent_id << ',' << event.depth << ',' << event.level << ','
        << event.bucket_id << ',' << event.size << ',' << event.radius << '\n';
  }
}

inline void export_delta_trace(const std::string& path, const Trace& trace) {
  std::ofstream out(path);
  out << "level,depth,base_delta,effective_delta,radius\n";
  for (const auto& event : trace.delta_events()) {
    out << event.level << ',' << event.depth << ',' << event.base_delta << ',' << event.effective_delta << ','
        << event.radius << '\n';
  }
}

}  // namespace analysis
