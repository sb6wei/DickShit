cmake_minimum_required(VERSION 3.16)
project(BMSSPFramework LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(bmssp_core
  src/algorithms/bmssp.cpp
  src/algorithms/dijkstra.cpp
)

target_include_directories(bmssp_core PUBLIC src)

add_executable(benchmark src/experiments/benchmark.cpp)
target_link_libraries(benchmark bmssp_core)

add_executable(experiment_runner src/experiments/experiment_runner.cpp)
target_link_libraries(experiment_runner bmssp_core)
