#ifndef HPROF_ANALYZER_SRC_MAIN_ARGS_H_
#define HPROF_ANALYZER_SRC_MAIN_ARGS_H_

#include <string>
#include <memory>

namespace hprof_analyzer {
typedef struct args {
  std::unique_ptr<std::string> log;
  std::unique_ptr<std::string> heap_dump;
} args_t;

args_t parse_args(int argc, char *argv[]);
}
#endif //HPROF_ANALYZER_SRC_MAIN_ARGS_H_
