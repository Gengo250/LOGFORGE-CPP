#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "log_entry.hpp"

namespace logforge {

struct LatencyStats {
  std::uint64_t count = 0;
  int min_ms = -1;
  int max_ms = -1;
  double avg_ms = 0.0;
  int p50_ms = -1;
  int p95_ms = -1;
  int p99_ms = -1;
};

struct Report {
  std::uint64_t total_lines = 0;
  std::uint64_t parsed_lines = 0;
  std::uint64_t invalid_lines = 0;

  std::unordered_map<int, std::uint64_t> status_counts;
  std::unordered_map<std::string, std::uint64_t> endpoint_counts;
  std::unordered_map<std::string, std::uint64_t> per_minute_counts;

  LatencyStats latency;
};

class Aggregator {
public:
  explicit Aggregator(int top_n = 20);

  void add_valid(const LogEntry& e);
  void add_invalid();

  // Finaliza e computa percentis de latência.
  Report finalize();

  int top_n() const { return top_n_; }

private:
  int top_n_;

  Report report_;

  // Latência por histograma (aproximado, mas streaming-friendly).
  static constexpr int kBucketMs = 50;
  static constexpr int kMaxMs = 10000;
  static constexpr int kBucketCount = (kMaxMs / kBucketMs) + 2; // + overflow
  std::vector<std::uint64_t> latency_hist_;
  std::uint64_t latency_sum_ms_ = 0;

  void add_latency(int latency_ms);
  int percentile_from_hist(double p) const;
};

} // namespace logforge
