#include "hprof_io.h"

#include <vector>
#include <list>
#include <unordered_map>
#include <algorithm>
#include <utility>

#include "types.h"
#include "log/log.h"

using std::string, std::unique_ptr, std::unordered_map, std::make_unique, hprof_analyzer::u1,
    hprof_analyzer::u2, hprof_analyzer::u4, hprof_analyzer::u8, std::vector,
    hprof_analyzer::constant_pool_record_t, hprof_analyzer::static_field_record_t,
    hprof_analyzer::field_record_t, hprof_analyzer::heap_region_info_t,
    hprof_analyzer::object_array_info_t, hprof_analyzer::primitive_array_info_t,
    hprof_analyzer::instance_info_t, hprof_analyzer::outgoing_reference_t, std::list;

static bool compare_region(const heap_region_info_t &region, const int64_t addr) {
  return region.bottom <= addr;
}

static uint8_t find_region_type(const std::vector<heap_region_info_t> &regions, int64_t addr) {
  const auto it = std::lower_bound(regions.begin(), regions.end(), addr, compare_region);

  if (it != regions.end() && it->bottom >= addr && addr < it->top) {
    return it->type;  // return index of found region
  }

  return -1;  // not found
}

static inline u1 read_u1(u1 **bytes_ptr) {
  u1 *bytes = *bytes_ptr;
  *bytes_ptr += 1;
  return *(reinterpret_cast<const u1 *>(bytes));
}

static inline u2 read_u2(u1 **bytes_ptr) {
  u1 *bytes = *bytes_ptr;
  *bytes_ptr += 2;
  return ntohs(*(reinterpret_cast<const u2 *>(bytes)));
}

static inline u4 read_u4(u1 **bytes_ptr) {
  u1 *bytes = *bytes_ptr;
  *bytes_ptr += 4;
  return ntohl(*(reinterpret_cast<const u4 *>(bytes)));
}

static inline u8 read_u8(u1 **bytes_ptr) {
  u1 *bytes = *bytes_ptr;
  *bytes_ptr += 8;
  return ntohll(*(reinterpret_cast<const u8 *>(bytes)));
}

static inline u8 parse_identifier(u1 **bytes, u4 identifier_size) {
  switch (identifier_size) {
    case 1: return read_u1(bytes);
    case 2: return read_u2(bytes);
    case 4: return read_u4(bytes);
    default: return read_u8(bytes);
  }
}

static inline u8 read_value(u1 **bytes_ptr, u1 type, u4 identifier_size) {
  switch (type) {
    case TYPE_ARRAY_OBJECT:
    case TYPE_NORMAL_OBJECT:return parse_identifier(bytes_ptr, identifier_size);
    case TYPE_BOOLEAN:return read_u1(bytes_ptr);
    case TYPE_CHAR:return ntohll(read_u2(bytes_ptr));
    case TYPE_FLOAT:return ntohll(read_u4(bytes_ptr));
    case TYPE_DOUBLE:return ntohll(read_u8(bytes_ptr));
    case TYPE_BYTE:return read_u1(bytes_ptr);
    case TYPE_SHORT:return ntohll(read_u2(bytes_ptr));
    case TYPE_INT:return ntohll(read_u4(bytes_ptr));
    case TYPE_LONG:return ntohll(read_u8(bytes_ptr));
    default:throw std::runtime_error("unknown value type:" + std::to_string(type));
  }
}

static inline u8 get_type_size(u1 type, u4 identifier_size) {
  switch (type) {
    case TYPE_ARRAY_OBJECT:
    case TYPE_NORMAL_OBJECT:return identifier_size;
    case TYPE_BOOLEAN:return 1;
    case TYPE_CHAR:return 2;
    case TYPE_FLOAT:return 4;
    case TYPE_DOUBLE:return 8;
    case TYPE_BYTE:return 1;
    case TYPE_SHORT:return 2;
    case TYPE_INT:return 4;
    case TYPE_LONG:return 8;
    default:throw std::runtime_error("unknown value type:" + std::to_string(type));
  }
}

static inline void parse_string(hprof_analyzer::HprofData *hprof, u1 *bytes, u4 size, u4 identifier_size) {
  u8 id = parse_identifier(&bytes, identifier_size);
  hprof->add_string(id, string(reinterpret_cast<char *>(bytes), size - identifier_size));
}

static inline void parse_heap_dump_segment_root_unknown(u1 **bytes,
                                                        u4 identifier_size,
                                                        hprof_analyzer::HprofData *hprof_data) {
  u8 id = parse_identifier(bytes, identifier_size);
  hprof_data->add_root(id);
  log("type: Root unknown, id = 0x" << std::hex << id)
}

static inline void parse_heap_dump_segment_root_jni_global(u1 **bytes,
                                                           u4 identifier_size,
                                                           hprof_analyzer::HprofData *hprof_data) {
  u8 id = parse_identifier(bytes, identifier_size);
  [[maybe_unused]] u8 jni_global_ref_id = parse_identifier(bytes, identifier_size);
  hprof_data->add_root(id);
  log("type: Root JNI global, id = 0x" << std::hex << id)
}

static inline void parse_heap_dump_segment_root_jni_local(u1 **bytes,
                                                          u4 identifier_size,
                                                          hprof_analyzer::HprofData *hprof_data) {
  u8 id = parse_identifier(bytes, identifier_size);
  [[maybe_unused]] u4 thead_serial_number = read_u4(bytes);
  hprof_data->add_root(id);
  [[maybe_unused]] u4 stack_trace_frame_number = read_u4(bytes);
  log("type: Root JNI local, id = 0x" << std::hex << id)
}

static inline void parse_heap_dump_segment_root_java_frame(u1 **bytes,
                                                           u4 identifier_size,
                                                           hprof_analyzer::HprofData *hprof_data) {
  u8 id = parse_identifier(bytes, identifier_size);
  [[maybe_unused]] u4 thead_serial_number = read_u4(bytes);
  hprof_data->add_root(id);
  [[maybe_unused]] u4 stack_trace_frame_number = read_u4(bytes);
  log("type: Root Java frame, id = 0x" << std::hex << id)
}

static inline void parse_heap_dump_segment_root_native_stack(u1 **bytes,
                                                             u4 identifier_size,
                                                             hprof_analyzer::HprofData *hprof_data) {
  u8 id = parse_identifier(bytes, identifier_size);
  [[maybe_unused]] u4 thead_serial_number = read_u4(bytes);
  hprof_data->add_root(id);
  log("type: Root native stack, id = 0x" << std::hex << id)
}

static inline void parse_heap_dump_segment_root_sticky_class(u1 **bytes,
                                                             u4 identifier_size,
                                                             hprof_analyzer::HprofData *hprof_data) {
  u8 id = parse_identifier(bytes, identifier_size);
  log("type: Root sticky class, id = 0x" << std::hex << id)
  hprof_data->add_root(id);
}

static inline void parse_heap_dump_segment_root_thread_block(u1 **bytes,
                                                             u4 identifier_size,
                                                             hprof_analyzer::HprofData *hprof_data) {
  u8 id = parse_identifier(bytes, identifier_size);
  [[maybe_unused]] u4 thead_serial_number = read_u4(bytes);
  hprof_data->add_root(id);
  log("type: Root thread block, id = 0x" << std::hex << id)
}

static inline void parse_heap_dump_segment_root_monitor_used(u1 **bytes,
                                                             u4 identifier_size,
                                                             hprof_analyzer::HprofData *hprof_data) {
  u8 id = parse_identifier(bytes, identifier_size);
  log("type: Root monitor used, id = 0x" << std::hex << id)
  hprof_data->add_root(id);
}

static inline void parse_heap_dump_segment_root_thread_object(u1 **bytes,
                                                              u4 identifier_size,
                                                              hprof_analyzer::HprofData *hprof_data) {
  u8 id = parse_identifier(bytes, identifier_size);
  [[maybe_unused]] u4 thead_serial_number = read_u4(bytes);
  hprof_data->add_root(id);
  [[maybe_unused]] u4 stack_trace_serial_number = read_u4(bytes);
  log("type: Root thread object, id = 0x" << std::hex << id)
}

static inline void parse_heap_dump_segment_class_dump(u1 **bytes,
                                                      u4 identifier_size,
                                                      hprof_analyzer::HprofData *hprof) {
  [[maybe_unused]] u8 id = parse_identifier(bytes, identifier_size);
  [[maybe_unused]] u4 stack_trace_serial_number = read_u4(bytes);
  [[maybe_unused]]u8 super_class_id = parse_identifier(bytes, identifier_size);
  [[maybe_unused]]u8 class_loader_id = parse_identifier(bytes, identifier_size);
  [[maybe_unused]]u8 signers_object_id = parse_identifier(bytes, identifier_size);
  [[maybe_unused]]u8 protection_domain_id = parse_identifier(bytes, identifier_size);
  *bytes += 2 * identifier_size;  // skip 2 reserved fields
  [[maybe_unused]]u4 instance_size = read_u4(bytes);
  [[maybe_unused]]u2 constant_pool_size = read_u2(bytes);
  int64_t class_size = 0;
  auto constant_pool_records = make_unique<vector<constant_pool_record_t>>(constant_pool_size);
  for (int i = 0; i < constant_pool_size; ++i) {
    constant_pool_record_t &record = constant_pool_records->at(i);
    record.index = read_u2(bytes);
    record.type = read_u1(bytes);
    class_size += get_type_size(record.type, identifier_size);
    record.value = read_value(bytes, record.type, identifier_size);
  }
  [[maybe_unused]]u2 static_fields_size = read_u2(bytes);
  [[maybe_unused]]auto static_fields = make_unique<vector<static_field_record_t>>(static_fields_size);
  for (int i = 0; i < static_fields_size; ++i) {
    static_field_record_t &record = static_fields->at(i);
    record.string_id = parse_identifier(bytes, identifier_size);
    record.type = read_u1(bytes);
    class_size += get_type_size(record.type, identifier_size);
    record.value = read_value(bytes, record.type, identifier_size);
  }
  [[maybe_unused]]u2 instance_fields_size = read_u2(bytes);
  [[maybe_unused]]auto instance_fields = make_unique<vector<field_record_t>>(instance_fields_size);
  for (int i = 0; i < instance_fields_size; ++i) {
    field_record_t &record = instance_fields->at(i);
    record.string_id = parse_identifier(bytes, identifier_size);
    record.type = read_u1(bytes);
  }
  hprof->add_class(id,
                   super_class_id,
                   class_size,
                   std::move(constant_pool_records),
                   std::move(static_fields),
                   std::move(instance_fields));
  log("type: class dump, id = 0x" << std::hex << id)
}

static inline void parse_heap_dump_segment_instance_dump(u1 **bytes,
                                                         u4 identifier_size,
                                                         unordered_map<int64_t, instance_info_t> *instances,
                                                         const vector<heap_region_info_t> &regions) {
  u8 id = parse_identifier(bytes, identifier_size);
  [[maybe_unused]] u4 stack_trace_serial_number = read_u4(bytes);
  [[maybe_unused]] u8 class_object_id = parse_identifier(bytes, identifier_size);
  u4 num_bytes = read_u4(bytes);
  [[maybe_unused]] u1 *fields = *bytes;

  *bytes += num_bytes;
  instances->insert({id,
                     instance_info_t{id, class_object_id, num_bytes, -1, 0, find_region_type(regions, id), false,
                                     fields,
                                     vector<outgoing_reference_t>()}});
  log("type: instance dump, id = 0x" << std::hex << id)
}

static inline void parse_heap_dump_segment_object_array_dump(u1 **bytes,
                                                             u4 identifier_size,
                                                             hprof_analyzer::HprofData *hprof,
                                                             const vector<heap_region_info_t> &regions) {
  u8 id = parse_identifier(bytes, identifier_size);
  [[maybe_unused]] u4 stack_trace_serial_number = read_u4(bytes);
  u4 elements_number = read_u4(bytes);
  u8 array_class_object_id = parse_identifier(bytes, identifier_size);
  int64_t size = elements_number * identifier_size;
  hprof->add_object_array(object_array_info_t{id, elements_number, size, array_class_object_id,
                                              find_region_type(regions, id), *bytes});
  *bytes += size;
  log("type: object array dump, id = 0x" << std::hex << id)
}

static inline void parse_heap_dump_segment_primitive_array_dump(u1 **bytes,
                                                                u4 identifier_size,
                                                                hprof_analyzer::HprofData *hprof,
                                                                const vector<heap_region_info_t> &regions) {
  [[maybe_unused]] u8 id = parse_identifier(bytes, identifier_size);
  [[maybe_unused]] u4 stack_trace_serial_number = read_u4(bytes);
  [[maybe_unused]] u4 elements_number = read_u4(bytes);
  [[maybe_unused]] u1 type = read_u1(bytes);
  int64_t size = elements_number * get_type_size(type, identifier_size);
  hprof->add_primitive_array(primitive_array_info_t{id, elements_number, size, type, find_region_type(regions, id),
                                                    *bytes});
  *bytes += size;
  log("type: primitive array dump, id = 0x" << std::hex << id)
}

static inline void parse_heap_dump_segment(u1 *bytes,
                                           u4 size,
                                           u4 identifier_size,
                                           hprof_analyzer::HprofData *hprof_data,
                                           unordered_map<int64_t, instance_info_t> *instances,
                                           const std::vector<heap_region_info_t> &regions) {
  u1 *end = bytes + size;
  while (bytes < end) {
    u1 type = read_u1(&bytes);
    if (type == HEAP_TAG_ROOT_UNKNOWN) {
      parse_heap_dump_segment_root_unknown(&bytes, identifier_size, hprof_data);
    } else if (type == HEAP_TAG_ROOT_JNI_GLOBAL) {
      parse_heap_dump_segment_root_jni_global(&bytes, identifier_size, hprof_data);
    } else if (type == HEAP_TAG_ROOT_JNI_LOCAL) {
      parse_heap_dump_segment_root_jni_local(&bytes, identifier_size, hprof_data);
    } else if (type == HEAP_TAG_ROOT_JAVA_FRAME) {
      parse_heap_dump_segment_root_java_frame(&bytes, identifier_size, hprof_data);
    } else if (type == HEAP_TAG_ROOT_NATIVE_STACK) {
      parse_heap_dump_segment_root_native_stack(&bytes, identifier_size, hprof_data);
    } else if (type == HEAP_TAG_ROOT_STICKY_CLASS) {
      parse_heap_dump_segment_root_sticky_class(&bytes, identifier_size, hprof_data);
    } else if (type == HEAP_TAG_ROOT_THREAD_BLOCK) {
      parse_heap_dump_segment_root_thread_block(&bytes, identifier_size, hprof_data);
    } else if (type == HEAP_TAG_ROOT_MONITOR_USED) {
      parse_heap_dump_segment_root_monitor_used(&bytes, identifier_size, hprof_data);
    } else if (type == HEAP_TAG_ROOT_THREAD_OBJ) {
      parse_heap_dump_segment_root_thread_object(&bytes, identifier_size, hprof_data);
    } else if (type == HEAP_TAG_CLASS_DUMP) {
      parse_heap_dump_segment_class_dump(&bytes, identifier_size, hprof_data);
    } else if (type == HEAP_TAG_INSTANCE_DUMP) {
      parse_heap_dump_segment_instance_dump(&bytes, identifier_size, instances, regions);
    } else if (type == HEAP_TAG_OBJ_ARRAY_DUMP) {
      parse_heap_dump_segment_object_array_dump(&bytes, identifier_size, hprof_data, regions);
    } else if (type == HEAP_TAG_PRIM_ARRAY_DUMP) {
      parse_heap_dump_segment_primitive_array_dump(&bytes, identifier_size, hprof_data, regions);
    } else {
      throw std::runtime_error("unknown heap dump type:" + std::to_string(type));
    }
  }
}

static inline bool try_add_array(int64_t root_ptr,
                                 hprof_analyzer::HprofData *hprof,
                                 unordered_map<int64_t, instance_info_t> *instances,
                                 instance_info_t *instance,
                                 list<instance_info_t *> *instance_queue,
                                 int64_t *outgoing_size,
                                 uint32_t identifier_size) {
  list<int64_t> array_elements_queue;
  array_elements_queue.push_back(root_ptr);
  bool added = false;
  while (!array_elements_queue.empty()) {
    auto current = array_elements_queue.front();
    array_elements_queue.pop_front();
    if (auto obj_array_ptr = hprof->get_object_array(current); obj_array_ptr != nullptr) {
      added = true;
      *outgoing_size += obj_array_ptr->size;
      auto array_ptr = obj_array_ptr->values;
      for (int64_t i = 0; i < obj_array_ptr->capacity; ++i) {
        auto obj_ref = parse_identifier(&array_ptr, identifier_size);
        if (obj_ref == 0) {
          // null pointer
        } else if (auto instance_search = instances->find(obj_ref); instance_search != instances->end()) {
          auto &outgoing_instance = instance_search->second;
          if (outgoing_instance.traverse_id == 0) {
            outgoing_instance.traverse_id = instance->traverse_id + 1;
          }
          instance_queue->push_back(&outgoing_instance);
        } else if (auto obj_array_array_ptr = hprof->get_object_array(obj_ref); obj_array_array_ptr != nullptr) {
          array_elements_queue.push_back(obj_array_array_ptr->id);
        } else if (auto primitive_array_ptr = hprof->get_primitive_array(obj_ref); primitive_array_ptr != nullptr) {
          *outgoing_size += primitive_array_ptr->size;
        }
      }
    }
  }
  return added;
}

static inline void scan_root(hprof_analyzer::HprofData *hprof,
                             uint32_t identifier_size,
                             unordered_map<int64_t, instance_info_t> *instances,
                             instance_info_t *instance_info) {
  list<instance_info_t *> queue;
  queue.push_back(instance_info);
  while (!queue.empty()) {
    auto instance = queue.front();
    queue.pop_front();
    instance->live = true;
    if (instance->outgoing_size > -1) continue;
    auto class_info = hprof->get_class(instance->class_id);

    auto fields_ptr = instance->all_fields;
    if (instance->outgoing_references.empty()) {
      while (class_info != nullptr) {
        for (auto &field : *class_info->instance_fields) {
          if (field.type == TYPE_NORMAL_OBJECT || field.type == TYPE_ARRAY_OBJECT) {
            instance->outgoing_references.push_back(outgoing_reference_t{hprof->get_string(field.string_id),
                                                                         parse_identifier(&fields_ptr,
                                                                                          identifier_size)});
          } else {
            fields_ptr += get_type_size(field.type, identifier_size);
          }
        }
        if (class_info->parent_id != 0) {
          class_info = hprof->get_class(class_info->parent_id);
        } else {
          class_info = nullptr;
        }
      }
    }
    if (instance->outgoing_references.empty()) {
      instance->outgoing_size = 0;
    } else {
      bool all_traversed = true;
      int64_t outgoing_size = 0;
      for (auto &ref : instance->outgoing_references) {
        if (ref.ptr == 0) {  // null ptr case
          outgoing_size += identifier_size;
        } else if (ref.ptr == instance->id) {  // self reference
          outgoing_size += identifier_size;
        } else if (auto outgoing_instance_search = instances->find(ref.ptr); outgoing_instance_search
            != instances->end()) {
          auto &outgoing_instance = outgoing_instance_search->second;
          if (outgoing_instance.outgoing_size == -1) {
            if (outgoing_instance.traverse_id == 0) {
              outgoing_instance.traverse_id = instance->traverse_id + 1;
            }
            if (outgoing_instance.traverse_id <= instance->traverse_id) continue;  // cyclic reference
            auto contains = (std::find(queue.begin(), queue.end(), &outgoing_instance) != queue.end());
            if (!contains) {
              queue.push_back(&outgoing_instance);
            }
            all_traversed = false;
          } else {
            outgoing_size += outgoing_instance.own_size + outgoing_instance.outgoing_size;
          }
        } else if (auto outgoing_class_search = hprof->get_classes()->find(ref.ptr); outgoing_class_search
            != hprof->get_classes()->end()) {
          // do not add statics size, just add size of the reference
          outgoing_size += identifier_size;
          for (auto &field : *outgoing_class_search->second.static_fields) {
            if (field.type == TYPE_NORMAL_OBJECT) {
              auto search = instances->find(field.value);
              if (search != instances->end()) {
                queue.push_back(&search->second);
              }
            }
          }
        } else if (try_add_array(ref.ptr, hprof, instances, instance, &queue, &outgoing_size, identifier_size)) {
        } else if (auto prim_array_ptr = hprof->get_primitive_array(ref.ptr); prim_array_ptr != nullptr) {
          outgoing_size += prim_array_ptr->size;
        } else {
          throw std::runtime_error(
              "not found reference " + *hprof->get_class(instance->class_id)->name_ptr + " " + *ref.name_ptr);
        }
      }
      if (all_traversed) {
        instance->outgoing_size = outgoing_size;
      } else {
        instance->outgoing_size = -1;
        queue.push_back(instance);
      }
    }
  }
}

unique_ptr<hprof_analyzer::HprofData> hprof_analyzer::read(unsigned char *buf,
                                                           const vector<heap_region_info_t> &regions) {
  auto hprof = make_unique<hprof_analyzer::HprofData>(hprof_analyzer::read_initial_header(&buf));
  auto identifier_size = hprof->get_identifier_size();
  auto instances = make_unique<unordered_map<int64_t, instance_info_t>>();
  while (*buf != 0) {
    u1 tag_type = read_u1(&buf);
    [[maybe_unused]] u4 ts = read_u4(&buf);
    u4 size = read_u4(&buf);
    u1 *bytes = buf;
    if (tag_type == TAG_HEAP_DUMP || tag_type == TAG_HEAP_DUMP_SEGMENT) {
      parse_heap_dump_segment(bytes, size, identifier_size, hprof.get(), instances.get(), regions);
    } else if (tag_type == TAG_UTF8) {
      parse_string(hprof.get(), bytes, size, identifier_size);
    } else if (tag_type == TAG_LOAD_CLASS) {
      [[maybe_unused]] auto serial_number = read_u4(&bytes);
      auto class_id = parse_identifier(&bytes, identifier_size);
      [[maybe_unused]] auto stack_trace_serial_number = read_u4(&bytes);
      auto class_name = parse_identifier(&bytes, identifier_size);
      hprof->add_class_string_id(class_id, class_name);
    }
    buf += size;
  }

  //  traverse classes
  for (auto &item : *hprof->get_classes()) {
    item.second.name_ptr = reinterpret_cast<string *>(hprof->get_string(hprof->get_class_string_id(item.second.id)));
    item.second.region_type = find_region_type(regions, item.second.id);
  }

  // calculate outgoing references
  for (auto &item : *instances) {
    if (hprof->is_root(item.first)) {
      scan_root(hprof.get(), identifier_size, instances.get(), &item.second);
    }
  }
  for (auto &class_info : *hprof->get_classes()) {
    if (hprof->is_root(class_info.first)) {
      for (auto &field : *class_info.second.static_fields) {
        if (field.type == TYPE_NORMAL_OBJECT) {
          auto search = instances->find(field.value);
          if (search != instances->end()) {
            scan_root(hprof.get(), identifier_size, instances.get(), &search->second);
          }
        }
      }
    }
  }
  for (auto &array_entry : *hprof->get_object_arrays()) {
    if (hprof->is_root(array_entry.first)) {
      auto array_ptr = array_entry.second.values;
      for (int64_t i = 0; i < array_entry.second.capacity; ++i) {
        auto obj_ref = parse_identifier(&array_ptr, identifier_size);
        if (auto instance_search = instances->find(obj_ref); instance_search != instances->end()) {
          scan_root(hprof.get(), identifier_size, instances.get(), &instance_search->second);
        }
      }
    }
  }
  for (auto &item : *instances) {
    if (item.second.outgoing_size == -1) {
      if (item.second.live) {
        throw std::runtime_error("Non traversed live object found");
      }
    }
  }
  hprof->set_instances(std::move(instances));
  return hprof;
}

unique_ptr<hprof_analyzer::header_t> hprof_analyzer::read_initial_header(unsigned char **buf_ptr) {
  char interim_buf[30];
  int i = 0;
  u1 value;
  do {
    value = read_u1(buf_ptr);
    interim_buf[i++] = static_cast<char>(value);
  } while (value != 0);
  interim_buf[i++] = 0;
  [[maybe_unused]] u4 identifiers_size = read_u4(buf_ptr);
  [[maybe_unused]] u8 timestamp = read_u8(buf_ptr);
  return make_unique<hprof_analyzer::header_t>(hprof_analyzer::header_t{string(interim_buf), identifiers_size,
                                                                        timestamp});
}
