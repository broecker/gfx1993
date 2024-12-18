cmake_minimum_required(VERSION 3.5.0)
project(gfx1993)

# The version number.
set(SRenderer_VERSION_MAJOR 0)
set(SRenderer_VERSION_MINOR 3)

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -g -DGLM_ENABLE_EXPERIMENTAL")

# Requirements
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
find_package(glm REQUIRED)

# The libraries
add_subdirectory(common)
add_subdirectory(geometry)
add_subdirectory(rendering)

# The examples
add_subdirectory(examples)
