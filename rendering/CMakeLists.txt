cmake_minimum_required (VERSION 2.6)
project (rendering)

# The version number.
set (rendering_VERSION_MAJOR 0)
set (rendering_VERSION_MINOR 2)

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -g -DGLM_ENABLE_EXPERIMENTAL")

# Main renderer library
add_library(rendering SHARED Rasteriser.cpp Framebuffer.cpp Depthbuffer.cpp Viewport.cpp Shader.cpp ShadingGeometry.cpp RenderPrimitive.cpp)