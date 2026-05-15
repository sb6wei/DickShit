#pragma once

#include <algorithm>
#include <cstddef>
#include <limits>
#include <vector>

namespace core {

class Distance {
 public:
  explicit Distance(std::size_t n) : values_(n, infinity()) {}

  static double infinity() { return std::numeric_limits<double>::infinity(); }

  double get(int node) const { return values_.at(static_cast<std::size_t>(node)); }
  void set(int node, double value) { values_.at(static_cast<std::size_t>(node)) = value; }

  std::size_t size() const { return values_.size(); }
  const std::vector<double>& values() const { return values_; }

 private:
  std::vector<double> values_;
};

}  // namespace core
