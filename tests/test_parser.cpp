#include <catch2/catch_test_macros.hpp>
#include "logforge/parser_nginx.hpp"

TEST_CASE("NginxParser parses valid line") {
  logforge::NginxParser p;

  const char* line =
      "127.0.0.1 - - [10/Oct/2000:13:55:36 -0700] "
      "\"GET /api/items?id=1 HTTP/1.1\" 200 2326 \"-\" \"Mozilla/5.0\" 0.245";

  auto e = p.parse_line(line);
  REQUIRE(e.has_value());
  CHECK(e->status == 200);
  CHECK(e->endpoint == "/api/items");
  CHECK(e->latency_ms == 245);
  CHECK(e->minute_key == "2000-10-10 13:55");
}

TEST_CASE("NginxParser rejects malformed line") {
  logforge::NginxParser p;
  auto e = p.parse_line("MALFORMED");
  CHECK_FALSE(e.has_value());
}
