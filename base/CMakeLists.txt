cmake_minimum_required(VERSION 3.5.0)

# Compile flag to enable experimental glm features.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_ENABLE_EXPERIMENTAL")

# Geometry library
add_library(gfx93-base STATIC config.h Pipeline.h Pipeline.cpp)
