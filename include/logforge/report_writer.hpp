#pragma once
#include <string>
#include "aggregator.hpp"

namespace logforge {

bool write_report_json(const Report& r, const std::string& out_dir, int top_n);
bool write_report_csv(const Report& r, const std::string& out_dir, int top_n);

} // namespace logforge
