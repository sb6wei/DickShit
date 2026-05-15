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

class Graph {
 public:
  Graph() = default;
  explicit Graph(std::size_t node_count) : adjacency_(node_count) {}

  std::size_t node_count() const { return adjacency_.size(); }
  std::size_t edge_count() const { return edge_count_; }

  const std::vector<Edge>& neighbors(int node) const { return adjacency_.at(node); }

  void add_edge(int from, int to, double weight) {
    if (from < 0 || to < 0) {
      return;
    }
    if (static_cast<std::size_t>(from) >= adjacency_.size() ||
        static_cast<std::size_t>(to) >= adjacency_.size()) {
      return;
    }
    adjacency_[from].push_back(Edge{to, weight, edge_count_++});
    max_edge_weight_ = std::max(max_edge_weight_, weight);
  }

  double max_edge_weight() const { return max_edge_weight_; }

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
    return graph;
  }

 private:
  std::vector<std::vector<Edge>> adjacency_;
  std::size_t edge_count_ = 0;
  double max_edge_weight_ = 0.0;
};

}  // namespace core
