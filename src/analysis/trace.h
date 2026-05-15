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

class Trace {
 public:
  void record_relax_event(int node, double old_distance, double new_distance, int level, int bucket_id) {
    relax_events_.push_back(RelaxEvent{node, old_distance, new_distance, level, bucket_id});
  }

  void record_distance(int node, double distance) { distance_events_.push_back(DistanceEvent{node, distance}); }

  const std::vector<RelaxEvent>& relax_events() const { return relax_events_; }
  const std::vector<DistanceEvent>& distance_events() const { return distance_events_; }

  void clear() {
    relax_events_.clear();
    distance_events_.clear();
  }

 private:
  std::vector<RelaxEvent> relax_events_;
  std::vector<DistanceEvent> distance_events_;
};

}  // namespace analysis
