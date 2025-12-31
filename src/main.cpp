#include <chrono>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "logforge/aggregator.hpp"
#include "logforge/buffered_reader.hpp"
#include "logforge/parser_nginx.hpp"
#include "logforge/report_writer.hpp"

using SteadyClock = std::chrono::steady_clock;

static void usage() {
  std::cout
      << "LogForge (starter)\n"
      << "Uso:\n"
      << "  logforge --in <arquivo.log> --out <diretorio_saida> [--top N] [--bench]\n\n"
      << "Exemplo:\n"
      << "  logforge --in data/sample_nginx.log --out out --top 20\n";
}

static std::string arg_value(const std::vector<std::string>& args, const std::string& key,
                             const std::string& def = "") {
  for (std::size_t i = 0; i + 1 < args.size(); ++i) {
    if (args[i] == key) return args[i + 1];
  }
  return def;
}

static bool has_flag(const std::vector<std::string>& args, const std::string& flag) {
  for (auto& a : args) if (a == flag) return true;
  return false;
}

static int arg_int(const std::vector<std::string>& args, const std::string& key, int def) {
  auto v = arg_value(args, key, "");
  if (v.empty()) return def;
  try { return std::stoi(v); } catch (...) { return def; }
}

int main(int argc, char** argv) {
  std::vector<std::string> args(argv + 1, argv + argc);

  if (has_flag(args, "--help") || has_flag(args, "-h") || args.empty()) {
    usage();
    return 0;
  }

  const std::string in_path = arg_value(args, "--in", "");
  const std::string out_dir = arg_value(args, "--out", "out");
  const int top_n = arg_int(args, "--top", 20);
  const bool bench = has_flag(args, "--bench");

  if (in_path.empty()) {
    std::cerr << "Erro: --in é obrigatório.\n\n";
    usage();
    return 2;
  }

  std::filesystem::create_directories(out_dir);

  logforge::BufferedLineReader reader(in_path);
  if (!reader.ok()) {
    std::cerr << "Erro: não foi possível abrir: " << in_path << "\n";
    return 2;
  }

  logforge::NginxParser parser;
  logforge::Aggregator agg(top_n);

  std::string line;
  auto t0 = SteadyClock::now();

  while (reader.next_line(line)) {
    auto entry = parser.parse_line(line);
    if (entry) agg.add_valid(*entry);
    else agg.add_invalid();
  }

  auto report = agg.finalize();
  auto t1 = SteadyClock::now();

  const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();
  const double sec = ms / 1000.0;
  const double lps = (sec > 0.0) ? (static_cast<double>(report.total_lines) / sec) : 0.0;

  if (bench) {
    std::cout << "BENCH\n";
    std::cout << "  linhas: " << report.total_lines << "\n";
    std::cout << "  invalidas: " << report.invalid_lines << "\n";
    std::cout << "  tempo: " << ms << " ms\n";
    std::cout << "  throughput: " << lps << " linhas/s\n";
    return 0;
  }

  // Escreve relatórios
  if (!logforge::write_report_json(report, out_dir, top_n)) {
    std::cerr << "Erro: falhou ao escrever report.json\n";
    return 3;
  }
  if (!logforge::write_report_csv(report, out_dir, top_n)) {
    std::cerr << "Erro: falhou ao escrever CSVs\n";
    return 3;
  }

  std::cout << "OK ✅\n";
  std::cout << "  total_lines=" << report.total_lines
            << " parsed=" << report.parsed_lines
            << " invalid=" << report.invalid_lines << "\n";
  std::cout << "  latency_ms: count=" << report.latency.count
            << " avg=" << report.latency.avg_ms
            << " p95~=" << report.latency.p95_ms << "\n";
  std::cout << "  wrote: " << out_dir << "/report.json + CSVs\n";
  std::cout << "  time: " << ms << " ms (" << lps << " linhas/s)\n";

  return 0;
}
