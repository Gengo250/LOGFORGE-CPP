#pragma once
#include <fstream>
#include <string>
#include <vector>

namespace logforge {

// Leitor de linhas com buffer grande (reduz overhead de I/O).
class BufferedLineReader {
public:
  explicit BufferedLineReader(const std::string& path, std::size_t buffer_size = 1 << 20);

  bool ok() const { return ifs_.is_open() && ifs_.good(); }
  bool next_line(std::string& out);

private:
  std::ifstream ifs_;
  std::vector<char> buffer_;
};

} // namespace logforge
