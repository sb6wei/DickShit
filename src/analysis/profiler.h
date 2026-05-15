#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

namespace analysis {

class Profiler {
 public:
  explicit Profiler(std::size_t edge_count = 0) : edge_usage_(edge_count, 0) {}

  void reset_edges(std::size_t edge_count) { edge_usage_.assign(edge_count, 0); }

  void reset_nodes(std::size_t node_count) {
    frontier_marker_.assign(node_count, 0);
    frontier_epoch_ = 1;
    previous_epoch_ = 0;
  }

  void record_edge(std::size_t edge_id) {
    if (edge_id < edge_usage_.size()) {
      ++edge_usage_[edge_id];
    }
  }

  template <typename NodeRange>
  void record_frontier(const NodeRange& nodes, std::size_t depth) {
    std::size_t size = static_cast<std::size_t>(nodes.size());
    frontier_sizes_.push_back(size);
    max_frontier_size_ = std::max(max_frontier_size_, size);
    ++frontier_count_;
    recursion_depths_.push_back(depth);
    max_recursion_depth_ = std::max(max_recursion_depth_, depth);

    if (!frontier_marker_.empty()) {
      std::size_t overlap = 0;
      for (int node : nodes) {
        std::size_t index = static_cast<std::size_t>(node);
        if (index < frontier_marker_.size() && frontier_marker_[index] == previous_epoch_) {
          ++overlap;
        }
        if (index < frontier_marker_.size()) {
          frontier_marker_[index] = frontier_epoch_;
        }
      }
      frontier_overlaps_.push_back(overlap);
      previous_epoch_ = frontier_epoch_;
      ++frontier_epoch_;
      if (frontier_epoch_ == std::numeric_limits<std::size_t>::max()) {
        std::fill(frontier_marker_.begin(), frontier_marker_.end(), 0);
        frontier_epoch_ = 1;
        previous_epoch_ = 0;
      }
    }
  }

  void record_frontier_node(int node) {
    frontier_sizes_.push_back(1);
    max_frontier_size_ = std::max<std::size_t>(max_frontier_size_, 1);
    ++frontier_count_;
    recursion_depths_.push_back(0);
    if (!frontier_marker_.empty()) {
      std::size_t overlap = 0;
      std::size_t index = static_cast<std::size_t>(node);
      if (index < frontier_marker_.size() && frontier_marker_[index] == previous_epoch_) {
        overlap = 1;
      }
      if (index < frontier_marker_.size()) {
        frontier_marker_[index] = frontier_epoch_;
      }
      frontier_overlaps_.push_back(overlap);
      previous_epoch_ = frontier_epoch_;
      ++frontier_epoch_;
    }
  }

  void record_delta(std::size_t level, std::size_t depth, double delta, double radius) {
    delta_levels_.push_back(level);
    delta_depths_.push_back(depth);
    delta_values_.push_back(delta);
    delta_radii_.push_back(radius);
  }

  void record_relax_attempt() { ++relax_attempts_; }
  void record_relax_success() { ++relax_successes_; }

  std::size_t relax_attempts() const { return relax_attempts_; }
  std::size_t relax_successes() const { return relax_successes_; }
  std::size_t max_frontier_size() const { return max_frontier_size_; }
  std::size_t max_recursion_depth() const { return max_recursion_depth_; }
  std::size_t frontier_count() const { return frontier_count_; }
  const std::vector<std::size_t>& edge_usage() const { return edge_usage_; }
  const std::vector<std::size_t>& frontier_sizes() const { return frontier_sizes_; }
  const std::vector<std::size_t>& frontier_overlaps() const { return frontier_overlaps_; }
  const std::vector<std::size_t>& recursion_depths() const { return recursion_depths_; }
  const std::vector<std::size_t>& delta_levels() const { return delta_levels_; }
  const std::vector<std::size_t>& delta_depths() const { return delta_depths_; }
  const std::vector<double>& delta_values() const { return delta_values_; }
  const std::vector<double>& delta_radii() const { return delta_radii_; }

 private:
  std::vector<std::size_t> edge_usage_;
  std::vector<std::size_t> frontier_sizes_;
  std::vector<std::size_t> frontier_overlaps_;
  std::vector<std::size_t> recursion_depths_;
  std::vector<std::size_t> delta_levels_;
  std::vector<std::size_t> delta_depths_;
  std::vector<double> delta_values_;
  std::vector<double> delta_radii_;
  std::vector<std::size_t> frontier_marker_;
  std::size_t relax_attempts_ = 0;
  std::size_t relax_successes_ = 0;
  std::size_t max_frontier_size_ = 0;
  std::size_t max_recursion_depth_ = 0;
  std::size_t frontier_count_ = 0;
  std::size_t frontier_epoch_ = 1;
  std::size_t previous_epoch_ = 0;
};

}  // namespace analysis
