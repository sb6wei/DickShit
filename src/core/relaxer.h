#pragma once

#include <cmath>
#include <limits>
#include <vector>

#include "distance.h"
#include "graph.h"
#include "scheduler.h"
#include "../analysis/profiler.h"
#include "../analysis/trace.h"

namespace core {

struct RelaxerConfig {
  bool enable_batching = true;
  bool enable_approximate = true;
  int propagation_radius = 1;
  int max_propagation_radius = 2;
  std::size_t edge_reuse_cap = 0;
};

class BatchedRelaxer {
 public:
  explicit BatchedRelaxer(std::size_t node_count = 0, std::size_t edge_count = 0) {
    reset(node_count, edge_count);
  }

  void reset(std::size_t node_count, std::size_t edge_count) {
    candidates_.assign(node_count, Distance::infinity());
    touched_.clear();
    touched_.reserve(node_count / 8 + 8);
    edge_epoch_.assign(edge_count, 0);
    edge_usage_guard_.assign(edge_count, 0);
    current_epoch_ = 1;
  }

  void configure(const RelaxerConfig& config) { config_ = config; }

  std::size_t memory_bytes() const {
    std::size_t bytes = 0;
    bytes += candidates_.capacity() * sizeof(double);
    bytes += touched_.capacity() * sizeof(int);
    bytes += edge_epoch_.capacity() * sizeof(std::size_t);
    bytes += edge_usage_guard_.capacity() * sizeof(std::size_t);
    return bytes;
  }

  template <typename NodeRange>
  void batch_relax(const Graph& graph,
                   Distance& distance,
                   const NodeRange& nodes,
                   std::size_t level,
                   int bucket_id,
                   double frontier_radius,
                   std::size_t depth,
                   HierarchicalBucketScheduler& scheduler,
                   analysis::Profiler* profiler,
                   analysis::Trace* trace) {
    if (nodes.empty()) {
      return;
    }
    if (candidates_.size() < graph.node_count()) {
      reset(graph.node_count(), graph.edge_count());
    }
    int local_radius = config_.propagation_radius;
    if (config_.enable_approximate && config_.max_propagation_radius > local_radius) {
      double delta = scheduler.delta(level);
      double scale = delta > 0.0 ? frontier_radius / delta : 0.0;
      local_radius =
          std::min(config_.max_propagation_radius, std::max(local_radius, static_cast<int>(std::ceil(scale))));
    }
    int min_bucket = std::max(0, bucket_id - local_radius);
    int max_bucket = bucket_id + local_radius;
    double delta = scheduler.delta(level);

    for (int node : nodes) {
      double base = distance.get(node);
      for (const auto& edge : graph.neighbors(node)) {
        if (edge.id < edge_epoch_.size()) {
          if (edge_epoch_[edge.id] == current_epoch_) {
            continue;
          }
          edge_epoch_[edge.id] = current_epoch_;
        }
        if (profiler) {
          profiler->record_edge(edge.id);
          profiler->record_relax_attempt();
        }
        if (config_.enable_approximate && config_.edge_reuse_cap > 0 &&
            edge.id < edge_usage_guard_.size() &&
            edge_usage_guard_[edge.id] >= config_.edge_reuse_cap) {
          continue;
        }
        double candidate = base + edge.weight;
        if (config_.enable_approximate && delta > 0.0) {
          int candidate_bucket = static_cast<int>(std::floor(candidate / delta));
          if (candidate_bucket < min_bucket || candidate_bucket > max_bucket) {
            continue;
          }
        }
        if (edge.id < edge_usage_guard_.size()) {
          ++edge_usage_guard_[edge.id];
        }

        if (!config_.enable_batching) {
          double current = distance.get(edge.to);
          if (candidate < current) {
            distance.set(edge.to, candidate);
            if (profiler) {
              profiler->record_relax_success();
            }
            if (trace) {
              trace->record_distance(edge.to, candidate);
              trace->record_relax_event(edge.to, current, candidate, level, bucket_id);
            }
            scheduler.add_node(level, edge.to, candidate);
          }
          continue;
        }

        if (candidate < candidates_[edge.to]) {
          if (candidates_[edge.to] == Distance::infinity()) {
            touched_.push_back(edge.to);
          }
          candidates_[edge.to] = candidate;
        }
      }
    }

    if (config_.enable_batching) {
      for (int node : touched_) {
        double candidate = candidates_[node];
        candidates_[node] = Distance::infinity();
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
      touched_.clear();
    }

    ++current_epoch_;
    if (current_epoch_ == std::numeric_limits<std::size_t>::max()) {
      std::fill(edge_epoch_.begin(), edge_epoch_.end(), 0);
      current_epoch_ = 1;
    }
  }

 private:
  RelaxerConfig config_;
  std::vector<double> candidates_;
  std::vector<int> touched_;
  std::vector<std::size_t> edge_epoch_;
  std::vector<std::size_t> edge_usage_guard_;
  std::size_t current_epoch_ = 1;
};

}  // namespace core
