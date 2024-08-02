#include "gtest/gtest.h"
#include <string>
#include <vector>
#include "generation/generation_analyzer.h"

class GenerationAnalyzerTest : public ::testing::Test {
 private:
  void add_expected_region(int64_t base, hprof_analyzer::generation_t type) {
    hprof_analyzer::heap_region_info_t region{};
    region.bottom = base;
    region.top = base + 0x1;
    region.end = base + 0x2;
    region.type = type;
    expected_regions.push_back(region);
  }
 protected:
  void SetUp() override {
    add_expected_region(0, hprof_analyzer::generation_t::YOUNG);
    add_expected_region(0x2, hprof_analyzer::generation_t::YOUNG);
    add_expected_region(0x4, hprof_analyzer::generation_t::OLD);
    add_expected_region(0x6, hprof_analyzer::generation_t::OLD);
    add_expected_region(0x8, hprof_analyzer::generation_t::FREE);
    add_expected_region(0xA, hprof_analyzer::generation_t::OTHER);
  }
  std::vector<hprof_analyzer::heap_region_info_t> expected_regions;
};

TEST_F(GenerationAnalyzerTest, ParseHeapRegionInfo) {
  char input[] = R"(Heap Regions: E=young(eden), S=young(survivor)
|   0|0x0, 0x1, 0x2| 99%| E|  |TAMS 0x00000007fe800000| PB 0x00000007fe800000| Untracked
|   0|0x2, 0x3, 0x4| 99%| S|  |TAMS 0x00000007fe800000| PB 0x00000007fe800000| Untracked
|   0|0x4, 0x5, 0x6| 99%| O|  |TAMS 0x00000007fe800000| PB 0x00000007fe800000| Untracked
|   0|0x6, 0x7, 0x8| 99%| HC|  |TAMS 0x00000007fe800000| PB 0x00000007fe800000| Untracked
|   0|0x8, 0x9, 0xA| 99%| F|  |TAMS 0x00000007fe800000| PB 0x00000007fe800000| Untracked
|   0|0xA, 0xB, 0xC| 99%| WTF|  |TAMS 0x00000007fe800000| PB 0x00000007fe800000| Untracked
)";
  auto regions = hprof_analyzer::parse_hear_region_info(input);
  ASSERT_TRUE(regions);
  ASSERT_EQ(expected_regions.size(), regions->size());
  for (size_t i = 0; i < expected_regions.size(); ++i) {
    EXPECT_EQ(expected_regions[i].bottom, (*regions)[i].bottom);
    EXPECT_EQ(expected_regions[i].top, (*regions)[i].top);
    EXPECT_EQ(expected_regions[i].end, (*regions)[i].end);
    EXPECT_EQ(expected_regions[i].type, (*regions)[i].type);
  }
}

TEST_F(GenerationAnalyzerTest, RegionsNotFound) {
  char input[] = "";
  bool failed = false;
  try {
    auto regions = hprof_analyzer::parse_hear_region_info(input);
  } catch (const std::runtime_error &e) {
    failed = true;
  }
  ASSERT_TRUE(failed);
}