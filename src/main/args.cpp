#include "args.h"

hprof_analyzer::args_t hprof_analyzer::parse_args(int argc, char *argv[]) {
  if (argc != 5) {
    throw std::runtime_error("Wrong number of arguments. Use -l log_file -h heap_dump");
  }
  hprof_analyzer::args_t result{};
  auto first_argument = std::string(argv[1]);
  auto third_argument = std::string(argv[3]);
  if (first_argument == "-l" && third_argument == "-h") {
    result.log = std::make_unique<std::string>(std::string(argv[2]));
    result.heap_dump = std::make_unique<std::string>(std::string(argv[4]));
  } else if (first_argument == "-h" && third_argument == "-l") {
    result.heap_dump = std::make_unique<std::string>(std::string(argv[2]));
    result.log = std::make_unique<std::string>(std::string(argv[4]));
  } else {
    throw std::runtime_error("Unknown argument: " + std::string(argv[1]) + ". Use -l log_file -h heap_dump");
  }
  return result;
}
