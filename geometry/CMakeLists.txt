cmake_minimum_required(VERSION 3.5.0)

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_ENABLE_EXPERIMENTAL")

# Geometry library
add_library(gfx93-geometry STATIC Geometry.h Geometry.cpp CubeGeometry.cpp RandomTriangleGeometry.cpp GridGeometry.cpp PlyGeometry.cpp Quad.cpp Quad.h Sphere.h Sphere.cpp Teapot.h Teapot.cpp Terrain.h Terrain.cpp)
target_link_libraries(gfx93-geometry gfx93-base)