#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace core {

struct Edge {
  int to = 0;
  double weight = 0.0;
  std::size_t id = 0;
};

struct EdgeSpan {
  const Edge* data = nullptr;
  std::size_t size = 0;

  const Edge* begin() const { return data; }
  const Edge* end() const { return data + size; }
};

class Graph {
 public:
  Graph() = default;
  explicit Graph(std::size_t node_count) : node_count_(node_count), staging_(node_count) {}

  std::size_t node_count() const { return node_count_; }
  std::size_t edge_count() const { return edge_count_; }

  EdgeSpan neighbors(int node) const {
    if (finalized_) {
      std::size_t index = static_cast<std::size_t>(node);
      std::size_t start = offsets_.at(index);
      std::size_t end = offsets_.at(index + 1);
      return EdgeSpan{edges_.data() + start, end - start};
    }
    const auto& bucket = staging_.at(static_cast<std::size_t>(node));
    return EdgeSpan{bucket.data(), bucket.size()};
  }

  void add_edge(int from, int to, double weight) {
    if (from < 0 || to < 0) {
      return;
    }
    if (static_cast<std::size_t>(from) >= node_count_ ||
        static_cast<std::size_t>(to) >= node_count_) {
      return;
    }
    staging_[static_cast<std::size_t>(from)].push_back(Edge{to, weight, edge_count_++});
    max_edge_weight_ = std::max(max_edge_weight_, weight);
  }

  void finalize() {
    if (finalized_) {
      return;
    }
    offsets_.assign(node_count_ + 1, 0);
    for (std::size_t i = 0; i < node_count_; ++i) {
      offsets_[i + 1] = offsets_[i] + staging_[i].size();
    }
    edges_.resize(edge_count_);
    for (std::size_t i = 0; i < node_count_; ++i) {
      std::size_t start = offsets_[i];
      std::copy(staging_[i].begin(), staging_[i].end(), edges_.begin() + static_cast<long>(start));
    }
    staging_.clear();
    staging_.shrink_to_fit();
    finalized_ = true;
  }

  double max_edge_weight() const { return max_edge_weight_; }

  std::size_t memory_bytes() const {
    std::size_t bytes = 0;
    bytes += edges_.capacity() * sizeof(Edge);
    bytes += offsets_.capacity() * sizeof(std::size_t);
    for (const auto& bucket : staging_) {
      bytes += bucket.capacity() * sizeof(Edge);
    }
    return bytes;
  }

  static Graph load_from_file(const std::string& path) {
    std::ifstream file(path);
    if (!file) {
      return Graph();
    }
    std::size_t n = 0;
    std::size_t m = 0;
    file >> n >> m;
    Graph graph(n);
    for (std::size_t i = 0; i < m; ++i) {
      int u = 0;
      int v = 0;
      double w = 0.0;
      file >> u >> v >> w;
      graph.add_edge(u, v, w);
    }
    graph.finalize();
    return graph;
  }

 private:
  std::size_t node_count_ = 0;
  std::vector<std::vector<Edge>> staging_;
  std::vector<Edge> edges_;
  std::vector<std::size_t> offsets_;
  std::size_t edge_count_ = 0;
  double max_edge_weight_ = 0.0;
  bool finalized_ = false;
};

}  // namespace core
