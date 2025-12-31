#include "logforge/report_writer.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <vector>

namespace logforge {

template <typename K, typename V>
static std::vector<std::pair<K, V>> to_vec(const std::unordered_map<K, V>& m) {
  std::vector<std::pair<K, V>> v;
  v.reserve(m.size());
  for (auto& kv : m) v.push_back(kv);
  return v;
}

static bool ensure_dir(const std::string& out_dir) {
  std::error_code ec;
  std::filesystem::create_directories(out_dir, ec);
  return !ec;
}

bool write_report_csv(const Report& r, const std::string& out_dir, int top_n) {
  if (!ensure_dir(out_dir)) return false;

  // status_counts.csv
  {
    std::string path = out_dir + "/status_counts.csv";
    std::ofstream ofs(path);
    if (!ofs.is_open()) return false;

    auto v = to_vec(r.status_counts);
    std::sort(v.begin(), v.end(), [](auto& a, auto& b) { return a.first < b.first; });

    ofs << "status,count\n";
    for (auto& kv : v) {
      ofs << kv.first << "," << kv.second << "\n";
    }
  }

  // top_endpoints.csv
  {
    std::string path = out_dir + "/top_endpoints.csv";
    std::ofstream ofs(path);
    if (!ofs.is_open()) return false;

    auto v = to_vec(r.endpoint_counts);
    std::sort(v.begin(), v.end(),
              [](auto& a, auto& b) { return (a.second == b.second) ? (a.first < b.first) : (a.second > b.second); });
    if (static_cast<int>(v.size()) > top_n) v.resize(static_cast<std::size_t>(top_n));

    ofs << "endpoint,count\n";
    for (auto& kv : v) {
      ofs << '"' << kv.first << '"' << "," << kv.second << "\n";
    }
  }

  // requests_per_minute.csv
  {
    std::string path = out_dir + "/requests_per_minute.csv";
    std::ofstream ofs(path);
    if (!ofs.is_open()) return false;

    auto v = to_vec(r.per_minute_counts);
    std::sort(v.begin(), v.end(), [](auto& a, auto& b) { return a.first < b.first; });

    ofs << "minute,count\n";
    for (auto& kv : v) {
      ofs << '"' << kv.first << '"' << "," << kv.second << "\n";
    }
  }

  // latency_summary.csv
  {
    std::string path = out_dir + "/latency_summary.csv";
    std::ofstream ofs(path);
    if (!ofs.is_open()) return false;

    ofs << "key,value\n";
    ofs << "count," << r.latency.count << "\n";
    ofs << "min_ms," << r.latency.min_ms << "\n";
    ofs << "avg_ms," << r.latency.avg_ms << "\n";
    ofs << "p50_ms," << r.latency.p50_ms << "\n";
    ofs << "p95_ms," << r.latency.p95_ms << "\n";
    ofs << "p99_ms," << r.latency.p99_ms << "\n";
    ofs << "max_ms," << r.latency.max_ms << "\n";
  }

  return true;
}

} // namespace logforge
