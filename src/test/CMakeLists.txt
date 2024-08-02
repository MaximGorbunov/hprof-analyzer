add_compile_options("-Werror" "-Wall" "-Wpedantic" "-Wextra" "-Wshadow" "-fexceptions" "-Wnon-virtual-dtor" "-Wno-unused-parameter" "-Wno-gnu")

add_link_options("-fsanitize=address;-fsanitize=undefined")

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/b796f7d44681514f58a683a3a71ff17c94edb0c1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

execute_process(COMMAND ${CMAKE_C_COMPILER} -print-file-name=${ASAN_NAME}
        OUTPUT_VARIABLE LIBASAN_PATH
        OUTPUT_STRIP_TRAILING_WHITESPACE)
include(GoogleTest)

set(CMAKE_TEST_RESOURCES_DIR "${PROJECT_SOURCE_DIR}/src/test/resources/")
configure_file(utils/config.h.in ${CMAKE_BINARY_DIR}/generated/config.h)
include_directories(${SRC_DIR} ${CMAKE_BINARY_DIR}/generated/)


add_executable(
        FileTest
        ${SRC_DIR}/io/file.cpp
        ${PROJECT_SOURCE_DIR}/src/test/io/file.cpp
)
target_link_libraries(
        FileTest
        GTest::gtest_main
)

gtest_discover_tests(FileTest)

add_executable(
        GenerationAnalyzerTest
        ${SRC_DIR}/generation/generation_analyzer.cpp
        ${PROJECT_SOURCE_DIR}/src/test/generation/generation_analyzer_test.cpp
)
target_link_libraries(
        GenerationAnalyzerTest
        GTest::gtest_main
)

gtest_discover_tests(GenerationAnalyzerTest)

add_executable(
        ArgsTest
        ${SRC_DIR}/args.cpp
        ${PROJECT_SOURCE_DIR}/src/test/args_test.cpp
)
target_link_libraries(
        ArgsTest
        GTest::gtest_main
)

gtest_discover_tests(ArgsTest)

add_executable(
        HprofParsingTest
        ${SRC_DIR}/hprof/hprof_io.cpp
        ${SRC_DIR}/hprof/hprof_data.cpp
        ${SRC_DIR}/io/file.cpp
        ${SRC_DIR}/generation/generation_analyzer.cpp
        ${PROJECT_SOURCE_DIR}/src/test/hprof/hprof_io_test.cpp
)
target_link_libraries(
        HprofParsingTest
        GTest::gtest_main
)

gtest_discover_tests(HprofParsingTest)