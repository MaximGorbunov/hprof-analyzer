#ifndef HPROF_ANALYZER_SRC_MAIN_GENERATION_GENERATIONANALYZER_H_
#define HPROF_ANALYZER_SRC_MAIN_GENERATION_GENERATIONANALYZER_H_

#include <vector>
#include <cstdint>
#include <memory>

namespace hprof_analyzer {
typedef enum generation {
  YOUNG, OLD, FREE, OTHER
} generation_t;

typedef struct heap_region_info {
  int64_t bottom;
  int64_t top;
  int64_t end;
  generation_t type;
} heap_region_info_t;

std::unique_ptr<std::vector<heap_region_info_t>> parse_hear_region_info(char *content);
}
#endif //HPROF_ANALYZER_SRC_MAIN_GENERATION_GENERATIONANALYZER_H_
