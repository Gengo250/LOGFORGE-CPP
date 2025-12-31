#pragma once
#include <optional>
#include <string_view>

#include "log_entry.hpp"

namespace logforge {

class Parser {
public:
  virtual ~Parser() = default;
  virtual std::optional<LogEntry> parse_line(std::string_view line) const = 0;
};

} // namespace logforge
