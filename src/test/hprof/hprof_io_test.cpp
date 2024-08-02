#include "args.h"
#include "gtest/gtest.h"

#include "hprof/hprof_io.h"
#include "io/file.h"

TEST(HprofParsingTest, IntegrationTest) {
  const hprof_analyzer::content_t &log = hprof_analyzer::read_file("../../../src/test/resources/example.log");
  const hprof_analyzer::content_t &heap = hprof_analyzer::read_file("../../../src/test/resources/example.hprof");
  const hprof_analyzer::content_t &expected = hprof_analyzer::read_file("../../../src/test/resources/expected.csv");
  auto regions = hprof_analyzer::parse_hear_region_info(reinterpret_cast<char *>(log.buffer.get()));
  auto hprof_data = hprof_analyzer::read(heap.buffer.get(), *regions);
  std::unordered_map<int64_t, hprof_analyzer::instance_info_t> *instances = hprof_data->get_instances();
  auto s = std::string(reinterpret_cast<char *>(expected.buffer.get()), expected.size);
  size_t pos = 0;
  size_t prev_pos = 0;
  while ((pos = s.find(',', pos)) != std::string::npos) {
    auto s1 = s.substr(prev_pos, pos);
    auto id = std::stoll(s1);
    prev_pos = pos;
    pos = s.find(',', pos + 1);
    auto own = std::stoll(s.substr(prev_pos + 1, pos - prev_pos - 1));
    prev_pos = pos;
    pos = s.find('\n', pos + 1);
    auto out = std::stoll(s.substr(prev_pos + 1, pos - prev_pos - 1));
    prev_pos = pos + 1;
    ASSERT_EQ(instances->at(id).own_size, own);
    ASSERT_EQ(instances->at(id).outgoing_size, out);
  }
  ASSERT_EQ(17522, instances->size());
}