#pragma once

#include <cmath>
#include <map>
#include <optional>
#include <vector>

#include "frontier.h"

namespace core {

class HierarchicalBucketScheduler {
 public:
  explicit HierarchicalBucketScheduler(std::vector<double> deltas)
      : deltas_(std::move(deltas)), buckets_(deltas_.size()) {}

  std::size_t level_count() const { return deltas_.size(); }

  double delta(std::size_t level) const { return deltas_.at(level); }

  void add_node(std::size_t level, int node, double distance) {
    if (level >= buckets_.size()) {
      return;
    }
    int bucket_id = static_cast<int>(std::floor(distance / deltas_[level]));
    buckets_[level][bucket_id].push_back(node);
  }

  bool has_work() const {
    for (const auto& level_buckets : buckets_) {
      if (!level_buckets.empty()) {
        return true;
      }
    }
    return false;
  }

  std::optional<Frontier> next_frontier() {
    for (std::size_t level = 0; level < buckets_.size(); ++level) {
      auto& level_buckets = buckets_[level];
      if (level_buckets.empty()) {
        continue;
      }
      auto iter = level_buckets.begin();
      Frontier frontier;
      frontier.level = static_cast<int>(level);
      frontier.bucket_id = iter->first;
      frontier.nodes = std::move(iter->second);
      level_buckets.erase(iter);
      return frontier;
    }
    return std::nullopt;
  }

 private:
  std::vector<double> deltas_;
  std::vector<std::map<int, std::vector<int>>> buckets_;
};

inline std::vector<double> build_deltas(double delta0, std::size_t max_levels, double min_delta) {
  std::vector<double> deltas;
  deltas.reserve(max_levels);
  double current = delta0;
  for (std::size_t i = 0; i < max_levels && current >= min_delta; ++i) {
    deltas.push_back(current);
    current = std::pow(current, 2.0 / 3.0);
  }
  return deltas;
}

}  // namespace core
