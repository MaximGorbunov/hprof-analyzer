#include <iostream>
#include <memory>
#include <algorithm>

#include "args.h"
#include "io/file.h"
#include "hprof/hprof_io.h"
#include "generation/generation_analyzer.h"
#include "output/hprof_data_print.h"

using hprof_analyzer::read_file, std::make_unique, hprof_analyzer::generation_t, hprof_analyzer::args_t,
    hprof_analyzer::parse_args,
    std::unique_ptr, std::string;

int main(int argc, char *argv[]) {
  const args_t &args = parse_args(argc, argv);
  const hprof_analyzer::content_t &hprof = read_file(*args.heap_dump);
  unsigned char *ptr = hprof.buffer.get();
  const hprof_analyzer::content_t &log_file = read_file(*args.log);
  auto regions = hprof_analyzer::parse_hear_region_info(reinterpret_cast<char *>(log_file.buffer.get()));
  auto hprof_data = hprof_analyzer::read(ptr, *regions);
  std::unordered_map<int64_t, hprof_analyzer::stats_t> stats;
  for (auto &item : *hprof_data->get_instances()) {
    auto &instance = item.second;
    if (item.second.region_type == hprof_analyzer::generation_t::OLD && item.second.live) {
      if (stats.contains(instance.class_id)) {
        stats[instance.class_id].own_size += instance.own_size;
        stats[instance.class_id].outgoing_size += instance.outgoing_size;
        stats[instance.class_id].total_size =
            stats[instance.class_id].own_size + stats[instance.class_id].outgoing_size;
      } else {
        stats[instance.class_id] =
            {instance.class_id, instance.own_size, instance.outgoing_size, instance.own_size + instance.outgoing_size};
      }
    }
  }
  std::vector<hprof_analyzer::stats_t> stats_vector;
  stats_vector.reserve(stats.size());
  for (auto &item : stats) {
    stats_vector.push_back(item.second);
  }
  std::sort(stats_vector.begin(),
            stats_vector.end(),
            [](const hprof_analyzer::stats_t &a, const hprof_analyzer::stats_t &b) {
              return a.total_size > b.total_size;
            });
  hprof_analyzer::print_hprof(stats_vector, hprof_data.get());
  return 0;
}
