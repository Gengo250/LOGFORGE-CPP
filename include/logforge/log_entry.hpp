#pragma once
#include <cstdint>
#include <string>

namespace logforge {

// Representa uma linha de log já normalizada.
struct LogEntry {
  std::string endpoint;       // ex: "/api/items" (sem querystring)
  int status = 0;             // ex: 200
  int latency_ms = -1;        // -1 se não houver
  std::string minute_key;     // ex: "2000-10-10 13:55" (para pico por minuto)
};

} // namespace logforge
