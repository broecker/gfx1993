cmake_minimum_required (VERSION 2.6)
project (common)

# The version number.
set (common_VERSION_MAJOR 0)
set (common_VERSION_MINOR 1)

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -g -DGLM_ENABLE_EXPERIMENTAL")

# Geometry library
add_library(common SHARED Camera.cpp)
