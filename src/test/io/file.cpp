#include <gtest/gtest.h>
#include <memory>
#include <iostream>

#include "io/file.h"
#include "config.h"

namespace hprof_analyzer {
using std::string, std::make_unique;
TEST(FileTest, ReadFile) {
  std::cout << CMAKE_TEST_RESOURCES_DIR << std::endl;
  string path = CMAKE_TEST_RESOURCES_DIR + "example.hprof";
  auto content = read_file(path);
  ASSERT_EQ(26633010, content.size);
}
}