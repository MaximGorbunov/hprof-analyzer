#ifndef HPROF_ANALYZER_SRC_MAIN_HPROF_HPROFDATA_H_
#define HPROF_ANALYZER_SRC_MAIN_HPROF_HPROFDATA_H_
#include <unordered_map>
#include <string>
#include <memory>
#include <unordered_set>
#include <vector>

#include "types.h"
#include "hprof_data.h"
#include "generation/generation_analyzer.h"

namespace hprof_analyzer {

typedef struct class_info {
  uint64_t id;
  std::string *name_ptr;
  uint64_t parent_id;
  uint8_t region_type;
  uint64_t outgoing_static_size;
  uint64_t own_size;
  std::unique_ptr<std::vector<constant_pool_record_t>> constant_pool;
  std::unique_ptr<std::vector<static_field_record_t>> static_fields;
  std::unique_ptr<std::vector<field_record_t>> instance_fields;
} class_info_t;

typedef struct outgoing_reference {
  std::string *name_ptr;
  uint64_t ptr;
} outgoing_reference_t;

typedef struct instance_info {
  uint64_t id;
  uint64_t class_id;
  uint64_t own_size;
  int64_t outgoing_size;
  uint64_t traverse_id;
  uint8_t region_type;
  bool live;
  u1 *all_fields;

  std::vector<outgoing_reference_t> outgoing_references;
} instance_info_t;

typedef struct primitive_array_info {
  uint64_t id;
  uint64_t capacity;
  uint64_t size;
  u1 type;
  uint8_t region_type;
  u1 *values;
} primitive_array_info_t;

typedef struct object_array_info {
  uint64_t id;
  uint64_t capacity;
  uint64_t size;
  uint64_t class_id;
  uint8_t region_type;
  u1 *values;
} object_array_info_t;

class HprofData {
 private:
  std::unordered_map<uint64_t, std::string> string_map_;
  std::unordered_map<uint64_t, uint64_t> class_string_id_map;
  std::unordered_map<uint64_t, hprof_analyzer::class_info_t> class_map_;
  std::unordered_map<uint64_t, hprof_analyzer::primitive_array_info_t> primitive_array_map_;
  std::unordered_map<uint64_t, hprof_analyzer::object_array_info_t> object_array_map_;
  std::unique_ptr<std::unordered_map<uint64_t, instance_info_t>> instances_;
  std::unordered_set<uint64_t> roots_;
  std::unique_ptr<header_t> header_;
 public:
  explicit HprofData(std::unique_ptr<header_t> header) : string_map_(), roots_(), header_(std::move(header)) {};
  std::string *get_string(uint64_t id);
  uint64_t get_class_string_id(uint64_t id);
  bool is_root(uint64_t id);
  void add_string(uint64_t id, const std::string &str);
  void add_class_string_id(uint64_t id, uint64_t class_string_id);
  class_info_t *get_class(uint64_t id);
  primitive_array_info_t *get_primitive_array(uint64_t id);
  object_array_info_t *get_object_array(uint64_t id);
  std::unordered_map<uint64_t, hprof_analyzer::object_array_info_t> *get_object_arrays();
  std::unordered_map<uint64_t, hprof_analyzer::primitive_array_info_t> *get_primitive_arrays();
  std::unordered_map<uint64_t, instance_info_t> *get_instances();
  void set_instances(std::unique_ptr<std::unordered_map<uint64_t, instance_info_t>> instances);
  void add_root(uint64_t id);
  void add_primitive_array(const primitive_array_info_t &record);
  void add_object_array(const object_array_info_t &record);
  void add_class(uint64_t id, uint64_t parent_id, uint64_t size,
                 std::unique_ptr<std::vector<constant_pool_record_t>> constant_pool,
                 std::unique_ptr<std::vector<static_field_record_t>> static_fields,
                 std::unique_ptr<std::vector<field_record_t>> instance_fields);
  u4 get_identifier_size();
  std::unordered_map<uint64_t, hprof_analyzer::class_info_t> *get_classes();
};
}

#endif //HPROF_ANALYZER_SRC_MAIN_HPROF_HPROFDATA_H_
