#pragma once

#include <algorithm>
#include <cstddef>
#include <vector>

namespace analysis {

class Profiler {
 public:
  explicit Profiler(std::size_t edge_count = 0) : edge_usage_(edge_count, 0) {}

  void reset_edges(std::size_t edge_count) { edge_usage_.assign(edge_count, 0); }

  void record_edge(std::size_t edge_id) {
    if (edge_id < edge_usage_.size()) {
      ++edge_usage_[edge_id];
    }
  }

  void record_frontier(std::size_t size) {
    frontier_sizes_.push_back(size);
    max_frontier_size_ = std::max(max_frontier_size_, size);
    ++frontier_count_;
  }

  void record_recursion_depth(std::size_t depth) {
    max_recursion_depth_ = std::max(max_recursion_depth_, depth);
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

 private:
  std::vector<std::size_t> edge_usage_;
  std::vector<std::size_t> frontier_sizes_;
  std::size_t relax_attempts_ = 0;
  std::size_t relax_successes_ = 0;
  std::size_t max_frontier_size_ = 0;
  std::size_t max_recursion_depth_ = 0;
  std::size_t frontier_count_ = 0;
};

}  // namespace analysis
