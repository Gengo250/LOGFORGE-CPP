#include "logforge/buffered_reader.hpp"

#include <utility>

namespace logforge {

BufferedLineReader::BufferedLineReader(const std::string& path, std::size_t buffer_size)
    : ifs_(path, std::ios::in), buffer_(buffer_size) {
  if (ifs_.is_open()) {
    // Configura buffer maior para reduzir chamadas ao SO.
    ifs_.rdbuf()->pubsetbuf(buffer_.data(), static_cast<std::streamsize>(buffer_.size()));
  }
}

bool BufferedLineReader::next_line(std::string& out) {
  if (!ifs_.good())
    return false;
  return static_cast<bool>(std::getline(ifs_, out));
}

} // namespace logforge
