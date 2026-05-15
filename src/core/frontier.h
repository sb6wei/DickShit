#pragma once

#include <vector>

namespace core {

struct Frontier {
  int level = 0;
  int bucket_id = 0;
  std::vector<int> nodes;
};

}  // namespace core
