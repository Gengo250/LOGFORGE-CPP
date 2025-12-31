#include <catch2/catch_test_macros.hpp>
#include "logforge/parser_nginx.hpp"

TEST_CASE("NginxParser parses line without request_time (latency missing)") {
  logforge::NginxParser p;

  const char* line =
      "127.0.0.1 - - [10/Oct/2000:13:55:36 -0700] "
      "\"GET /health HTTP/1.1\" 304 0 \"-\" \"curl/8.0\"";

  auto e = p.parse_line(line);
  REQUIRE(e.has_value());
  CHECK(e->status == 304);
  CHECK(e->endpoint == "/health");
  CHECK(e->latency_ms == -1);
  CHECK(e->minute_key == "2000-10-10 13:55");
}

TEST_CASE("NginxParser parses IPv6 + query + trailing spaces") {
  logforge::NginxParser p;

  const char* line =
      "2001:db8::1 - - [01/Jan/2025:00:00:01 -0300] "
      "\"GET /search?q=abc HTTP/1.1\" 200 123 \"-\" \"Mozilla/5.0\" 0.010   ";

  auto e = p.parse_line(line);
  REQUIRE(e.has_value());
  CHECK(e->status == 200);
  CHECK(e->endpoint == "/search");
  CHECK(e->latency_ms == 10);
  CHECK(e->minute_key == "2025-01-01 00:00");
}

TEST_CASE("NginxParser parses absolute URL in request path") {
  logforge::NginxParser p;

  const char* line =
      "127.0.0.1 - - [01/Jan/2025:00:00:59 -0300] "
      "\"GET https://example.com/api/items?id=9 HTTP/1.1\" 200 10 \"-\" \"Mozilla/5.0\" 0.123";

  auto e = p.parse_line(line);
  REQUIRE(e.has_value());
  CHECK(e->status == 200);
  CHECK(e->endpoint == "/api/items");
  CHECK(e->latency_ms == 123);
  CHECK(e->minute_key == "2025-01-01 00:00");
}

