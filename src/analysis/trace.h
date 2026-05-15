#pragma once

#include <cstddef>
#include <vector>

namespace analysis {

struct RelaxEvent {
  int node = 0;
  double old_distance = 0.0;
  double new_distance = 0.0;
  int level = 0;
  int bucket_id = 0;
};

struct DistanceEvent {
  int node = 0;
  double distance = 0.0;
};

struct RecursionEvent {
  std::size_t id = 0;
  std::size_t parent_id = 0;
  std::size_t depth = 0;
  std::size_t level = 0;
  int bucket_id = 0;
  std::size_t size = 0;
  double radius = 0.0;
};

struct DeltaEvent {
  std::size_t level = 0;
  std::size_t depth = 0;
  double base_delta = 0.0;
  double effective_delta = 0.0;
  double radius = 0.0;
};

class Trace {
 public:
  void record_relax_event(int node, double old_distance, double new_distance, int level, int bucket_id) {
    relax_events_.push_back(RelaxEvent{node, old_distance, new_distance, level, bucket_id});
  }

  void record_distance(int node, double distance) { distance_events_.push_back(DistanceEvent{node, distance}); }

  std::size_t record_recursion_event(std::size_t parent_id,
                                     std::size_t depth,
                                     std::size_t level,
                                     int bucket_id,
                                     std::size_t size,
                                     double radius) {
    std::size_t id = next_recursion_id_++;
    recursion_events_.push_back(RecursionEvent{id, parent_id, depth, level, bucket_id, size, radius});
    return id;
  }

  void record_delta_event(std::size_t level,
                          std::size_t depth,
                          double base_delta,
                          double effective_delta,
                          double radius) {
    delta_events_.push_back(DeltaEvent{level, depth, base_delta, effective_delta, radius});
  }

  const std::vector<RelaxEvent>& relax_events() const { return relax_events_; }
  const std::vector<DistanceEvent>& distance_events() const { return distance_events_; }
  const std::vector<RecursionEvent>& recursion_events() const { return recursion_events_; }
  const std::vector<DeltaEvent>& delta_events() const { return delta_events_; }

  void clear() {
    relax_events_.clear();
    distance_events_.clear();
    recursion_events_.clear();
    delta_events_.clear();
    next_recursion_id_ = 1;
  }

 private:
  std::vector<RelaxEvent> relax_events_;
  std::vector<DistanceEvent> distance_events_;
  std::vector<RecursionEvent> recursion_events_;
  std::vector<DeltaEvent> delta_events_;
  std::size_t next_recursion_id_ = 1;
};

}  // namespace analysis
