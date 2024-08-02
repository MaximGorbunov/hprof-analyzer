#include <filesystem>
#include <fstream>
#include <utility>

#include "file.h"

hprof_analyzer::content_t hprof_analyzer::read_file(const std::string &path) {
  std::filesystem::path inputFilePath = std::filesystem::absolute(std::filesystem::path{path});
  auto length = std::filesystem::file_size(inputFilePath);
  if (length == 0) {
    return {0, std::make_unique<unsigned char[]>(0)};  // empty vector
  }
  auto buffer = std::make_unique<unsigned char[]>(length);
  std::ifstream inputFile(path, std::ios_base::binary);
  inputFile.read(reinterpret_cast<char *>(buffer.get()), static_cast<std::streamsize>(length));
  inputFile.close();
  content_t content(length, std::move(buffer));
  return content;
}
