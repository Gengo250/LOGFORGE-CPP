#include "logforge/report_writer.hpp"

#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>

namespace logforge {

static std::string json_escape(const std::string& s) {
  std::string out;
  out.reserve(s.size() + 8);
  for (char c : s) {
    switch (c) {
      case '\\': out += "\\\\"; break;
      case '"': out += "\\\""; break;
      case '\n': out += "\\n"; break;
      case '\r': out += "\\r"; break;
      case '\t': out += "\\t"; break;
      default:
        if (static_cast<unsigned char>(c) < 0x20) {
          // omite controle (simples)
        } else {
          out += c;
        }
    }
  }
  return out;
}

template <typename K, typename V>
static std::vector<std::pair<K, V>> to_vec(const std::unordered_map<K, V>& m) {
  std::vector<std::pair<K, V>> v;
  v.reserve(m.size());
  for (auto& kv : m) v.push_back(kv);
  return v;
}

bool write_report_json(const Report& r, const std::string& out_dir, int top_n) {
  std::string path = out_dir;
  if (!path.empty() && path.back() != '/') path += '/';
  path += "report.json";

  std::ofstream ofs(path);
  if (!ofs.is_open()) return false;

  auto status = to_vec(r.status_counts);
  std::sort(status.begin(), status.end(), [](auto& a, auto& b) { return a.first < b.first; });

  auto endpoints = to_vec(r.endpoint_counts);
  std::sort(endpoints.begin(), endpoints.end(),
            [](auto& a, auto& b) { return (a.second == b.second) ? (a.first < b.first) : (a.second > b.second); });
  if (static_cast<int>(endpoints.size()) > top_n) endpoints.resize(static_cast<std::size_t>(top_n));

  auto minutes = to_vec(r.per_minute_counts);
  std::sort(minutes.begin(), minutes.end(), [](auto& a, auto& b) { return a.first < b.first; });

  std::ostringstream ss;
  ss << "{\n";
  ss << "  \"summary\": {\n";
  ss << "    \"total_lines\": " << r.total_lines << ",\n";
  ss << "    \"parsed_lines\": " << r.parsed_lines << ",\n";
  ss << "    \"invalid_lines\": " << r.invalid_lines << "\n";
  ss << "  },\n";

  ss << "  \"latency_ms\": {\n";
  ss << "    \"count\": " << r.latency.count << ",\n";
  ss << "    \"min\": " << r.latency.min_ms << ",\n";
  ss << "    \"avg\": " << r.latency.avg_ms << ",\n";
  ss << "    \"p50\": " << r.latency.p50_ms << ",\n";
  ss << "    \"p95\": " << r.latency.p95_ms << ",\n";
  ss << "    \"p99\": " << r.latency.p99_ms << ",\n";
  ss << "    \"max\": " << r.latency.max_ms << "\n";
  ss << "  },\n";

  ss << "  \"status_counts\": [\n";
  for (std::size_t i = 0; i < status.size(); ++i) {
    ss << "    {\"status\": " << status[i].first << ", \"count\": " << status[i].second << "}";
    ss << (i + 1 < status.size() ? "," : "") << "\n";
  }
  ss << "  ],\n";

  ss << "  \"top_endpoints\": [\n";
  for (std::size_t i = 0; i < endpoints.size(); ++i) {
    ss << "    {\"endpoint\": \"" << json_escape(endpoints[i].first) << "\", \"count\": " << endpoints[i].second
       << "}";
    ss << (i + 1 < endpoints.size() ? "," : "") << "\n";
  }
  ss << "  ],\n";

  ss << "  \"requests_per_minute\": [\n";
  for (std::size_t i = 0; i < minutes.size(); ++i) {
    ss << "    {\"minute\": \"" << json_escape(minutes[i].first) << "\", \"count\": " << minutes[i].second << "}";
    ss << (i + 1 < minutes.size() ? "," : "") << "\n";
  }
  ss << "  ]\n";

  ss << "}\n";

  ofs << ss.str();
  return true;
}

} // namespace logforge
