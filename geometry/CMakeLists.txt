cmake_minimum_required (VERSION 2.6)
project (geometry)

# The version number.
set (geometry_VERSION_MAJOR 0)
set (geometry_VERSION_MINOR 1)

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -g -DGLM_ENABLE_EXPERIMENTAL")

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

# Geometry library
add_library(geometry SHARED Geometry2.cpp CubeGeometry.cpp RandomTriangleGeometry.cpp GridGeometry.cpp PlyGeometry.cpp)
