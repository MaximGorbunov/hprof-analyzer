#ifndef HPROF_ANALYZER_SRC_MAIN_OUTPUT_HPROF_DATA_PRINT_H_
#define HPROF_ANALYZER_SRC_MAIN_OUTPUT_HPROF_DATA_PRINT_H_
#include "hprof/hprof_data.h"
namespace hprof_analyzer {

typedef struct stats {
  int64_t id;
  int64_t own_size;
  int64_t outgoing_size;
  int64_t total_size;
} stats_t;

void print_hprof(const std::vector<hprof_analyzer::stats_t> &stats_vector, HprofData *hprof_data);
}
#endif //HPROF_ANALYZER_SRC_MAIN_OUTPUT_HPROF_DATA_PRINT_H_
