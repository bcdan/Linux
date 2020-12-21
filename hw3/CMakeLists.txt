cmake_minimum_required(VERSION 3.17)
project(hw3 C)
set(GCC_COVERAGE_COMPILE_FLAGS "−Wall −lm")

set(CMAKE_C_STANDARD 99)
#add_executable(hw3  Clerk.c Sorter.c random.c stopwatch.c Sim.c stop.c)
add_executable(hw3  Sim.c stopwatch.c random.c)


