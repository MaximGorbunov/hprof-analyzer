#include "generation_analyzer.h"
#include <string>
#include <memory>
#include <algorithm>

using hprof_analyzer::heap_region_info_t, std::string, std::vector, std::unique_ptr, std::make_unique;

static hprof_analyzer::generation_t parse_generation_type(string *str) {
  str->erase(std::remove_if(str->begin(), str->end(), ::isspace), str->end());
  // Heap Regions: E=young(eden), S=young(survivor), O=old, HS=humongous(starts),
  // HC=humongous(continues), CS=collection set, F=free
  if (*str == "E" || *str == "S") return hprof_analyzer::generation_t::YOUNG;
  else if (*str == "O" || *str == "HC") return hprof_analyzer::generation_t::OLD;
  else if (*str == "F") return hprof_analyzer::generation_t::FREE;
  else
    return hprof_analyzer::generation_t::OTHER;
}

bool compareRegion(const heap_region_info_t &a, const heap_region_info_t &b) {
  return a.bottom < b.bottom;
}

unique_ptr<vector<heap_region_info_t>> hprof_analyzer::parse_hear_region_info(char *content) {
  char buffer[1024]{0};
  int buffer_position = 0;
  char last_char;
  bool heap_regions_start = false;
  auto regions = make_unique<vector<heap_region_info>>();
  do {
    last_char = *(content++);
    buffer[buffer_position++] = last_char;
    if (heap_regions_start && buffer_position == 1 && last_char != '|') {
      break;
    } else if (last_char == '\n') {
      string line(buffer);
      if (!heap_regions_start && line.starts_with("Heap Regions: E=young(eden), S=young(survivor)")) {
        heap_regions_start = true;
      } else if (heap_regions_start) {
        auto start = line.find("0x", 1);
        auto end = line.find('|', start);
        auto range_str = line.substr(start, end - start);
        uint64_t position = 0;
        uint64_t end_position = range_str.find(", ");
        heap_region_info region{};
        region.bottom = std::stoll(range_str.substr(position, end_position - position), nullptr, 16);
        position = end_position + 2;
        end_position = range_str.find(", ", position);
        region.top = std::stoll(range_str.substr(position, end_position - position), nullptr, 16);
        position = end_position + 2;
        region.end = std::stoll(range_str.substr(position), nullptr, 16);

        // parse type
        start = line.find('|', line.find('|', end + 1));
        end = line.find('|', start + 1);
        auto type = line.substr(start + 1, end - start - 1);
        region.type = parse_generation_type(&type);
        regions->push_back(region);
      }
      buffer_position = 0;
      memset(buffer, 0, sizeof(buffer));
    }
  } while (last_char != 0);
  if (regions->empty()) {
    throw std::runtime_error("Regions not found");
  }
  regions->shrink_to_fit();
  std::sort(regions->begin(), regions->end(), compareRegion);
  return regions;
}
