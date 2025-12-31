#include <catch2/catch_test_macros.hpp>
#include "logforge/aggregator.hpp"

TEST_CASE("Aggregator counts status and endpoints") {
  logforge::Aggregator agg(10);

  logforge::LogEntry a{"/a", 200, 100, "2025-01-01 00:00"};
  logforge::LogEntry b{"/b", 404, 200, "2025-01-01 00:00"};
  logforge::LogEntry c{"/a", 200, 300, "2025-01-01 00:01"};

  agg.add_valid(a);
  agg.add_valid(b);
  agg.add_valid(c);
  agg.add_invalid();

  auto r = agg.finalize();

  CHECK(r.total_lines == 4);
  CHECK(r.parsed_lines == 3);
  CHECK(r.invalid_lines == 1);

  CHECK(r.status_counts.at(200) == 2);
  CHECK(r.status_counts.at(404) == 1);
  CHECK(r.endpoint_counts.at("/a") == 2);
  CHECK(r.endpoint_counts.at("/b") == 1);
  CHECK(r.per_minute_counts.at("2025-01-01 00:00") == 2);
  CHECK(r.per_minute_counts.at("2025-01-01 00:01") == 1);

  CHECK(r.latency.count == 3);
  CHECK(r.latency.min_ms == 100);
  CHECK(r.latency.max_ms == 300);
}
