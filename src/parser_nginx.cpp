#include "logforge/parser_nginx.hpp"

#include <charconv>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <string>

namespace logforge {

static inline std::string_view ltrim(std::string_view sv) {
  while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.front()))) sv.remove_prefix(1);
  return sv;
}

static inline std::string_view rtrim(std::string_view sv) {
  while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.back()))) sv.remove_suffix(1);
  return sv;
}

static inline std::string_view trim(std::string_view sv) { return rtrim(ltrim(sv)); }

static inline std::string_view last_token(std::string_view sv) {
  sv = rtrim(sv);
  auto sp = sv.rfind(' ');
  if (sp == std::string_view::npos) return sv;
  return sv.substr(sp + 1);
}

static inline bool parse_int_sv(std::string_view sv, int& out) {
  sv = trim(sv);
  if (sv.empty()) return false;
  auto b = sv.data();
  auto e = sv.data() + sv.size();
  auto res = std::from_chars(b, e, out);
  return res.ec == std::errc() && res.ptr == e;
}

static inline bool parse_double_sv(std::string_view sv, double& out) {
  sv = trim(sv);
  if (sv.empty()) return false;

#if defined(__cpp_lib_to_chars) && (__cpp_lib_to_chars >= 201611L)
  auto b = sv.data();
  auto e = sv.data() + sv.size();
  auto res = std::from_chars(b, e, out, std::chars_format::general);
  return res.ec == std::errc() && res.ptr == e;
#else
  // Fallback (menos ideal): precisa de string null-terminated
  std::string tmp(sv);
  char* endp = nullptr;
  out = std::strtod(tmp.c_str(), &endp);
  return endp && *endp == '\0';
#endif
}

std::string NginxParser::strip_query(std::string_view path) {
  auto q = path.find('?');
  if (q == std::string_view::npos) return std::string(path);
  return std::string(path.substr(0, q));
}

int NginxParser::month_to_int(std::string_view mon) {
  if (mon == "Jan") return 1;
  if (mon == "Feb") return 2;
  if (mon == "Mar") return 3;
  if (mon == "Apr") return 4;
  if (mon == "May") return 5;
  if (mon == "Jun") return 6;
  if (mon == "Jul") return 7;
  if (mon == "Aug") return 8;
  if (mon == "Sep") return 9;
  if (mon == "Oct") return 10;
  if (mon == "Nov") return 11;
  if (mon == "Dec") return 12;
  return 0;
}

std::optional<std::string> NginxParser::parse_minute_key(std::string_view t) {
  // "10/Oct/2000:13:55:36 -0700" -> "2000-10-10 13:55"
  auto slash1 = t.find('/');
  if (slash1 == std::string_view::npos) return std::nullopt;
  auto slash2 = t.find('/', slash1 + 1);
  if (slash2 == std::string_view::npos) return std::nullopt;
  auto colon1 = t.find(':', slash2 + 1);
  if (colon1 == std::string_view::npos) return std::nullopt;

  std::string_view day_sv = t.substr(0, slash1);
  std::string_view mon_sv = t.substr(slash1 + 1, slash2 - (slash1 + 1));
  std::string_view year_sv = t.substr(slash2 + 1, colon1 - (slash2 + 1));

  // HH:MM (logo depois do primeiro ':')
  if (colon1 + 4 >= t.size()) return std::nullopt;
  std::string_view hh_sv = t.substr(colon1 + 1, 2);
  std::string_view mm_sv = t.substr(colon1 + 4, 2);

  int day = 0, year = 0, hh = 0, mm = 0;
  if (!parse_int_sv(day_sv, day)) return std::nullopt;
  if (!parse_int_sv(year_sv, year)) return std::nullopt;
  if (!parse_int_sv(hh_sv, hh)) return std::nullopt;
  if (!parse_int_sv(mm_sv, mm)) return std::nullopt;

  int mon = month_to_int(mon_sv);
  if (mon == 0) return std::nullopt;

  char buf[17];  // "YYYY-MM-DD HH:MM" + '\0' = 17
  std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d", year, mon, day, hh, mm);
  return std::string(buf);
}

std::optional<LogEntry> NginxParser::parse_line(std::string_view line) const {
  // 1) timestamp entre [ ... ]
  auto lb = line.find('[');
  if (lb == std::string_view::npos) return std::nullopt;

  auto rb = line.find(']', lb + 1);
  if (rb == std::string_view::npos || rb <= lb + 1) return std::nullopt;

  std::string_view time_sv = line.substr(lb + 1, rb - (lb + 1));
  auto minute_key = parse_minute_key(time_sv);
  if (!minute_key) return std::nullopt;

  // 2) request entre " ... " (primeiro par de aspas após ])
  auto q1 = line.find('"', rb);
  if (q1 == std::string_view::npos) return std::nullopt;
  auto q2 = line.find('"', q1 + 1);
  if (q2 == std::string_view::npos || q2 <= q1 + 1) return std::nullopt;

  std::string_view request = line.substr(q1 + 1, q2 - (q1 + 1));
  request = trim(request);

  // request esperado: METHOD PATH PROTO
  auto sp1 = request.find(' ');
  auto sp2 = request.rfind(' ');
  if (sp1 == std::string_view::npos || sp2 == std::string_view::npos || sp1 == sp2) return std::nullopt;

  std::string_view path_sv = request.substr(sp1 + 1, sp2 - (sp1 + 1));
  path_sv = trim(path_sv);
  if (path_sv.empty()) return std::nullopt;

  // Se vier URL absoluta, pega só o path
  if (path_sv.rfind("http://", 0) == 0 || path_sv.rfind("https://", 0) == 0) {
    auto scheme = path_sv.find("://");
    if (scheme != std::string_view::npos) {
      auto after = scheme + 3;
      auto slash = path_sv.find('/', after);
      path_sv = (slash == std::string_view::npos) ? std::string_view("/") : path_sv.substr(slash);
    }
  }

  std::string endpoint = strip_query(path_sv);
  if (endpoint.empty()) endpoint = std::string{"/"};

  // 3) status logo após o segundo quote
  std::string_view rest = line.substr(q2 + 1);
  rest = ltrim(rest);

  auto sp = rest.find(' ');
  std::string_view status_sv = (sp == std::string_view::npos) ? rest : rest.substr(0, sp);

  int status = 0;
  if (!parse_int_sv(status_sv, status)) return std::nullopt;

  // 4) latência: tenta parsear o último token como double (segundos)
  int latency_ms = -1;
  {
    std::string_view tok = last_token(line);
    double sec = 0.0;
    if (parse_double_sv(tok, sec)) {
      if (sec >= 0.0 && sec < 3600.0) latency_ms = static_cast<int>(sec * 1000.0);
    }
  }

  LogEntry e;
  e.endpoint = std::move(endpoint);
  e.status = status;
  e.latency_ms = latency_ms;
  e.minute_key = std::move(*minute_key);
  return e;
}

}  // namespace logforge
