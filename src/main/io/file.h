#ifndef HPROF_ANALYZER_SRC_MAIN_IO_FILE_H_
#define HPROF_ANALYZER_SRC_MAIN_IO_FILE_H_

#include <memory>
#include <string>

namespace hprof_analyzer {
typedef struct content {
  content(std::size_t _size, std::unique_ptr<unsigned char[]> content_buffer)
      : size(_size), buffer(std::move(content_buffer)) {}
  std::size_t size;
  std::unique_ptr<unsigned char[]> buffer;
} content_t;

content_t read_file(const std::string &path);
}
#endif //HPROF_ANALYZER_SRC_MAIN_IO_FILE_H_
