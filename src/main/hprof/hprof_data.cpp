#include "hprof_data.h"

#include <utility>

using hprof_analyzer::u4, hprof_analyzer::class_info_t, std::vector, std::unique_ptr,
    hprof_analyzer::primitive_array_info_t, hprof_analyzer::object_array_info_t,
    hprof_analyzer::instance_info_t, std::unordered_map;

void hprof_analyzer::HprofData::add_string(int64_t id, const std::string &str) {
  this->string_map_.insert({id, str});
}

std::string *hprof_analyzer::HprofData::get_string(int64_t id) {
  return &string_map_.at(id);
}

void hprof_analyzer::HprofData::add_class_string_id(int64_t id, int64_t class_string_id) {
  class_string_id_map.insert({id, class_string_id});
}

int64_t hprof_analyzer::HprofData::get_class_string_id(int64_t id) {
  return class_string_id_map.at(id);
}

class_info_t *hprof_analyzer::HprofData::get_class(int64_t id) {
  return &class_map_.at(id);
}

u4 hprof_analyzer::HprofData::get_identifier_size() {
  return header_->identifiers_size;
}

void hprof_analyzer::HprofData::add_root(int64_t id) {
  roots_.insert(id);
}

bool hprof_analyzer::HprofData::is_root(int64_t id) {
  return roots_.contains(id);
}

void hprof_analyzer::HprofData::add_class(int64_t id, int64_t parent_id, int64_t size,
                                          unique_ptr<vector<constant_pool_record_t>> constant_pool,
                                          unique_ptr<vector<static_field_record_t>> static_fields,
                                          unique_ptr<vector<field_record_t>> instance_fields) {
  class_map_.insert({id, class_info_t{id, 0, parent_id, 0, 0, size, std::move(constant_pool), std::move(static_fields),
                                      std::move(instance_fields)}});
}

std::unordered_map<int64_t, hprof_analyzer::class_info_t> *hprof_analyzer::HprofData::get_classes() {
  return &class_map_;
}

void hprof_analyzer::HprofData::add_primitive_array(const primitive_array_info_t &record) {
  primitive_array_map_.insert({record.id, record});
}
void hprof_analyzer::HprofData::add_object_array(const object_array_info_t &record) {
  object_array_map_.insert({record.id, record});
}

primitive_array_info_t *hprof_analyzer::HprofData::get_primitive_array(int64_t id) {
  if (auto search = primitive_array_map_.find(id); search != primitive_array_map_.end()) {
    return &search->second;
  }
  return nullptr;
}

object_array_info_t *hprof_analyzer::HprofData::get_object_array(int64_t id) {
  if (auto search = object_array_map_.find(id); search != object_array_map_.end()) {
    return &search->second;
  }
  return nullptr;
}

std::unordered_map<int64_t, hprof_analyzer::object_array_info_t> *hprof_analyzer::HprofData::get_object_arrays() {
  return &object_array_map_;
}

std::unordered_map<int64_t,
                   hprof_analyzer::primitive_array_info_t> *hprof_analyzer::HprofData::get_primitive_arrays() {
  return &primitive_array_map_;
}

void hprof_analyzer::HprofData::set_instances(std::unique_ptr<std::unordered_map<int64_t,
                                                                                 instance_info_t>> instances) {
  instances_ = std::move(instances);
}

unordered_map<int64_t, instance_info_t> *hprof_analyzer::HprofData::get_instances() {
  return instances_.get();
}
