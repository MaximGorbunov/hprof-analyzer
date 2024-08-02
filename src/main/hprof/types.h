#ifndef HPROF_ANALYZER_SRC_MAIN_HPROF_TYPES_H_
#define HPROF_ANALYZER_SRC_MAIN_HPROF_TYPES_H_
#include <cstdint>
#include <string>

namespace hprof_analyzer {
typedef uint8_t u1;
typedef uint16_t u2;
typedef uint32_t u4;
typedef uint64_t u8;

typedef struct header {
  std::string info;
  u4 identifiers_size;
  u8 timestamp_ms;
} header_t;

typedef struct record {
  u1 tag_type;
  u4 timestamp;
  u4 size;
  u1 *bytes;
} record_t;

typedef struct constant_pool_record {
  u2 index;
  u1 type;
  u8 value;
} constant_pool_record_t;

typedef struct static_field_record {
  u8 string_id;
  u1 type;
  u8 value;
} static_field_record_t;

typedef struct field_record {
  u8 string_id;
  u1 type;
} field_record_t;

//HPROF TAGS
#define TAG_UTF8              0x01
#define TAG_LOAD_CLASS        0x02
#define TAG_UNLOAD_CLASS      0x03
#define TAG_FRAME             0x04
#define TAG_TRACE             0x05
#define TAG_ALLOC_SITES       0x06
#define TAG_HEAP_SUMMARY      0x07
#define TAG_START_THREAD      0x0A
#define TAG_END_THREAD        0x0B
#define TAG_HEAP_DUMP         0x0C
#define TAG_CPU_SAMPLES       0x0D
#define TAG_CONTROL_SETTINGS  0x0E
#define TAG_HEAP_DUMP_SEGMENT 0x1C
#define TAG_HEAP_DUMP_END     0x2C

// Heap dump tags
#define HEAP_TAG_ROOT_UNKNOWN        0xFF
#define HEAP_TAG_ROOT_JNI_GLOBAL     0x01
#define HEAP_TAG_ROOT_JNI_LOCAL      0x02
#define HEAP_TAG_ROOT_JAVA_FRAME     0x03
#define HEAP_TAG_ROOT_NATIVE_STACK   0x04
#define HEAP_TAG_ROOT_STICKY_CLASS   0x05
#define HEAP_TAG_ROOT_THREAD_BLOCK   0x06
#define HEAP_TAG_ROOT_MONITOR_USED   0x07
#define HEAP_TAG_ROOT_THREAD_OBJ     0x08
#define HEAP_TAG_CLASS_DUMP          0x20
#define HEAP_TAG_INSTANCE_DUMP       0x21
#define HEAP_TAG_OBJ_ARRAY_DUMP      0x22
#define HEAP_TAG_PRIM_ARRAY_DUMP     0x23

#define TYPE_ARRAY_OBJECT   1
#define TYPE_NORMAL_OBJECT  2
#define TYPE_BOOLEAN        4
#define TYPE_CHAR           5
#define TYPE_FLOAT          6
#define TYPE_DOUBLE         7
#define TYPE_BYTE           8
#define TYPE_SHORT          9
#define TYPE_INT            10
#define TYPE_LONG           11

}
#endif //HPROF_ANALYZER_SRC_MAIN_HPROF_TYPES_H_
