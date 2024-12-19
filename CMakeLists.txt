cmake_minimum_required(VERSION 3.5.0)
project(gfx1993)

# The version number.
set(gfx1993_VERSION_MAJOR 0)
set(gfx1993_VERSION_MINOR 4)

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -DGLM_ENABLE_EXPERIMENTAL")
set(CMAKE_CXX_STANDARD 14)

# Requirements
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
find_package(glm REQUIRED)
find_package(absl CONFIG REQUIRED)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

# The libraries
add_subdirectory(common)
add_subdirectory(geometry)
add_subdirectory(rendering)

# The examples
add_subdirectory(examples)
