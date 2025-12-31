#pragma once
#include <optional>
#include <string_view>

#include "parser.hpp"

namespace logforge {

// Parser para um formato comum do Nginx:
// 127.0.0.1 - - [10/Oct/2000:13:55:36 -0700] "GET /path?x=1 HTTP/1.1" 200 2326 "-" "UA" 0.245
// (Último token opcional = request_time em segundos)
class NginxParser final : public Parser {
public:
  std::optional<LogEntry> parse_line(std::string_view line) const override;

private:
  static std::string strip_query(std::string_view path);
  static std::optional<std::string> parse_minute_key(std::string_view bracket_time);
  static int month_to_int(std::string_view mon);
};

} // namespace logforge
