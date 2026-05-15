#pragma once

#include <cmath>
#include <cstddef>
#include <limits>
#include <memory_resource>
#include <optional>
#include <vector>

#include "frontier.h"

namespace core {

struct DeltaConfig {
  double min_delta = 1e-6;
  double adaptive_shrink = 0.85;
  double depth_shrink = 0.12;
  double radius_shrink = 0.6;
  double level_radius_scale = 0.25;
  bool enable_adaptive = true;
};

class DeltaController {
 public:
  DeltaController(std::vector<double> deltas, DeltaConfig config)
      : deltas_(std::move(deltas)), config_(config) {}

  std::size_t level_count() const { return deltas_.size(); }

  double base_delta(std::size_t level) const { return deltas_.at(level); }

  double effective_delta(std::size_t level, std::size_t depth, double radius) const {
    double base = base_delta(level);
    if (!config_.enable_adaptive) {
      return base;
    }
    double depth_factor = 1.0 / (1.0 + config_.depth_shrink * static_cast<double>(depth));
    double level_factor = 1.0 / (1.0 + config_.level_radius_scale * static_cast<double>(level));
    double radius_factor = 1.0;
    if (radius > 0.0) {
      double target = base * (1.0 + config_.radius_shrink * static_cast<double>(level));
      radius_factor = std::min(1.0, radius / std::max(target, 1e-9));
    }
    double scaled = base * config_.adaptive_shrink * depth_factor * radius_factor * level_factor;
    return std::max(config_.min_delta, scaled);
  }

 private:
  std::vector<double> deltas_;
  DeltaConfig config_;
};

class RadixBucketQueue {
 public:
  RadixBucketQueue(double delta, std::size_t window, std::pmr::memory_resource* resource)
      : delta_(delta),
        window_(std::max<std::size_t>(window, 8)),
        resource_(resource ? resource : std::pmr::get_default_resource()) {}

  double delta() const { return delta_; }

  bool empty() const { return non_empty_ == 0; }

  void add_node(int bucket_id, int node) {
    ensure_bucket(bucket_id);
    std::size_t offset = static_cast<std::size_t>(bucket_id - base_bucket_);
    std::size_t index = (base_index_ + offset) % buckets_.size();
    if (bucket_ids_[index] == kEmptyBucket) {
      bucket_ids_[index] = bucket_id;
      ++non_empty_;
    }
    buckets_[index].push_back(node);
  }

  bool pop_frontier(Frontier& frontier) {
    if (non_empty_ == 0) {
      return false;
    }
    std::size_t scanned = 0;
    while (scanned < buckets_.size()) {
      std::size_t index = base_index_;
      if (bucket_ids_[index] != kEmptyBucket && !buckets_[index].empty()) {
        frontier.bucket_id = base_bucket_;
        frontier.nodes = std::move(buckets_[index]);
        buckets_[index] = NodeList(resource_);
        bucket_ids_[index] = kEmptyBucket;
        --non_empty_;
        return true;
      }
      advance_base();
      ++scanned;
    }
    return false;
  }

  std::size_t memory_bytes() const {
    std::size_t bytes = buckets_.capacity() * sizeof(NodeList);
    bytes += bucket_ids_.capacity() * sizeof(int);
    for (const auto& bucket : buckets_) {
      bytes += bucket.capacity() * sizeof(int);
    }
    return bytes;
  }

 private:
  void ensure_bucket(int bucket_id) {
    if (buckets_.empty()) {
      initialize(bucket_id);
      return;
    }
    while (bucket_id < base_bucket_ ||
           bucket_id >= base_bucket_ + static_cast<int>(buckets_.size())) {
      grow_to_fit(bucket_id);
    }
  }

  void initialize(int bucket_id) {
    buckets_.reserve(window_);
    bucket_ids_.assign(window_, kEmptyBucket);
    for (std::size_t i = 0; i < window_; ++i) {
      buckets_.emplace_back(resource_);
    }
    base_bucket_ = bucket_id;
    base_index_ = 0;
    non_empty_ = 0;
  }

  void grow_to_fit(int bucket_id) {
    std::size_t new_size = buckets_.empty() ? window_ : buckets_.size();
    while (bucket_id < base_bucket_ ||
           bucket_id >= base_bucket_ + static_cast<int>(new_size)) {
      new_size *= 2;
    }
    rebuild(new_size);
  }

  void rebuild(std::size_t new_size) {
    std::vector<NodeList> new_buckets;
    new_buckets.reserve(new_size);
    for (std::size_t i = 0; i < new_size; ++i) {
      new_buckets.emplace_back(resource_);
    }
    std::vector<int> new_bucket_ids(new_size, kEmptyBucket);
    for (std::size_t i = 0; i < bucket_ids_.size(); ++i) {
      if (bucket_ids_[i] == kEmptyBucket) {
        continue;
      }
      int bucket_id = bucket_ids_[i];
      std::size_t offset = static_cast<std::size_t>(bucket_id - base_bucket_);
      new_buckets[offset] = std::move(buckets_[i]);
      new_bucket_ids[offset] = bucket_id;
    }
    buckets_ = std::move(new_buckets);
    bucket_ids_ = std::move(new_bucket_ids);
    base_index_ = 0;
  }

  void advance_base() {
    buckets_[base_index_].clear();
    bucket_ids_[base_index_] = kEmptyBucket;
    base_index_ = (base_index_ + 1) % buckets_.size();
    ++base_bucket_;
  }

  static constexpr int kEmptyBucket = std::numeric_limits<int>::min();

  double delta_ = 0.0;
  std::size_t window_ = 0;
  std::vector<NodeList> buckets_;
  std::vector<int> bucket_ids_;
  std::size_t base_index_ = 0;
  int base_bucket_ = 0;
  std::size_t non_empty_ = 0;
  std::pmr::memory_resource* resource_ = nullptr;
};

class HierarchicalBucketScheduler {
 public:
  HierarchicalBucketScheduler(std::vector<double> deltas,
                              std::size_t bucket_window,
                              std::size_t node_count)
      : deltas_(std::move(deltas)),
        node_pool_storage_(std::max<std::size_t>(node_count * sizeof(int) * 4, 4096)),
        node_pool_(node_pool_storage_.data(),
                   node_pool_storage_.size(),
                   std::pmr::get_default_resource()) {
    levels_.reserve(deltas_.size());
    for (double delta : deltas_) {
      levels_.emplace_back(delta, bucket_window, &node_pool_);
    }
  }

  std::size_t level_count() const { return deltas_.size(); }

  double delta(std::size_t level) const { return deltas_.at(level); }

  void add_node(std::size_t level, int node, double distance) {
    if (level >= levels_.size()) {
      return;
    }
    int bucket_id = static_cast<int>(std::floor(distance / deltas_[level]));
    levels_[level].add_node(bucket_id, node);
  }

  bool has_work() const {
    for (const auto& level : levels_) {
      if (!level.empty()) {
        return true;
      }
    }
    return false;
  }

  std::optional<Frontier> next_frontier() {
    for (std::size_t level = 0; level < levels_.size(); ++level) {
      if (levels_[level].empty()) {
        continue;
      }
      Frontier frontier(&node_pool_);
      frontier.level = static_cast<int>(level);
      if (levels_[level].pop_frontier(frontier)) {
        return frontier;
      }
    }
    return std::nullopt;
  }

  std::size_t memory_bytes() const {
    std::size_t bytes = node_pool_storage_.capacity();
    for (const auto& level : levels_) {
      bytes += level.memory_bytes();
    }
    return bytes;
  }

 private:
  std::vector<double> deltas_;
  std::vector<std::byte> node_pool_storage_;
  std::pmr::monotonic_buffer_resource node_pool_;
  std::vector<RadixBucketQueue> levels_;
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
