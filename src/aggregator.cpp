#include "logforge/aggregator.hpp"

#include <algorithm>
#include <cmath>

namespace logforge {

Aggregator::Aggregator(int top_n) : top_n_(top_n), latency_hist_(kBucketCount, 0) {}

void Aggregator::add_invalid() {
  report_.total_lines++;
  report_.invalid_lines++;
}

void Aggregator::add_latency(int latency_ms) {
  if (latency_ms < 0) return;

  report_.latency.count++;
  latency_sum_ms_ += static_cast<std::uint64_t>(latency_ms);

  if (report_.latency.min_ms == -1 || latency_ms < report_.latency.min_ms) report_.latency.min_ms = latency_ms;
  if (report_.latency.max_ms == -1 || latency_ms > report_.latency.max_ms) report_.latency.max_ms = latency_ms;

  int idx = latency_ms / kBucketMs;
  if (idx < 0) idx = 0;
  if (idx >= kBucketCount - 1) idx = kBucketCount - 1; // overflow bucket
  latency_hist_[static_cast<std::size_t>(idx)]++;
}

void Aggregator::add_valid(const LogEntry& e) {
  report_.total_lines++;
  report_.parsed_lines++;

  report_.status_counts[e.status]++;
  report_.endpoint_counts[e.endpoint]++;
  if (!e.minute_key.empty()) report_.per_minute_counts[e.minute_key]++;

  add_latency(e.latency_ms);
}

int Aggregator::percentile_from_hist(double p) const {
  if (report_.latency.count == 0) return -1;
  if (p <= 0.0) return report_.latency.min_ms;
  if (p >= 1.0) return report_.latency.max_ms;

  const auto target = static_cast<std::uint64_t>(std::ceil(p * report_.latency.count));
  std::uint64_t cum = 0;
  for (std::size_t i = 0; i < latency_hist_.size(); ++i) {
    cum += latency_hist_[i];
    if (cum >= target) {
      int ms = static_cast<int>(i) * kBucketMs;
      return ms;
    }
  }
  return report_.latency.max_ms;
}

Report Aggregator::finalize() {
  if (report_.latency.count > 0) {
    report_.latency.avg_ms = static_cast<double>(latency_sum_ms_) / static_cast<double>(report_.latency.count);
    report_.latency.p50_ms = percentile_from_hist(0.50);
    report_.latency.p95_ms = percentile_from_hist(0.95);
    report_.latency.p99_ms = percentile_from_hist(0.99);
  }
  return report_;
}

} // namespace logforge
