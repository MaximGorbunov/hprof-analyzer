cmake_minimum_required(VERSION 3.28)
project(hprof_analyzer)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_C_STANDARD_REQUIRED True)

enable_testing()

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src/main/)

add_subdirectory(src/main ./build)
add_subdirectory(src/test)
