#pragma once

#include <memory_resource>
#include <vector>

namespace core {

using NodeList = std::pmr::vector<int>;

struct Frontier {
  int level = 0;
  int bucket_id = 0;
  NodeList nodes;

  explicit Frontier(std::pmr::memory_resource* resource = std::pmr::get_default_resource())
      : nodes(resource) {}
};

}  // namespace core
