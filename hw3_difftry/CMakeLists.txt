cmake_minimum_required(VERSION 3.17)
project(hw3_difftry C)

set(CMAKE_C_STANDARD 11)
set(GCC_COVERAGE_COMPILE_FLAGS "-lm")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}" )
add_executable(hw3_difftry stopwatch.c simulation.c random.c)
target_link_libraries(hw3_difftry m)