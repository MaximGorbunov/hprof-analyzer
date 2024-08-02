#ifndef HPROF_ANALYZER_SRC_MAIN_HPROF_HPROF_IO_H_
#define HPROF_ANALYZER_SRC_MAIN_HPROF_HPROF_IO_H_
#include <memory>
#include <string>

#include "types.h"
#include "hprof_data.h"
#include "generation/generation_analyzer.h"

namespace hprof_analyzer {
std::unique_ptr<HprofData> read(unsigned char *buf, const std::vector<hprof_analyzer::heap_region_info_t> &regions);
std::unique_ptr<hprof_analyzer::header_t> read_initial_header(unsigned char **buf_ptr);
}
#endif //HPROF_ANALYZER_SRC_MAIN_HPROF_HPROF_IO_H_
