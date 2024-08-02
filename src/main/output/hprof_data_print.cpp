#include "hprof_data_print.h"

#include <iostream>

void hprof_analyzer::print_hprof(const std::vector<hprof_analyzer::stats_t> &stats_vector, HprofData *hprof_data) {
  for (const auto &item : stats_vector) {
    std::cout << *hprof_data->get_class(item.id)->name_ptr << " own size: " << item.own_size << " outgoing size: "
              << item.outgoing_size << " total size: " << item.total_size << std::endl;
  }
}