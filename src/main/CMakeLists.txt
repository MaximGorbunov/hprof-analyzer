add_compile_options(
        "-O3" "-Werror" "-Wall" "-Wpedantic" "-Wextra" "-Wshadow" "-fexceptions"
        "-Wnon-virtual-dtor" "-Wno-unused-parameter"
)

include_directories(./)

file(GLOB_RECURSE SOURCES_CPP *.cpp)

add_executable(hprof_analyzer ${SOURCES_CPP})