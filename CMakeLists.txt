cmake_minimum_required (VERSION 2.6)
project (SRenderer)

# The version number.
set (SRenderer_VERSION_MAJOR 0)
set (SRenderer_VERSION_MINOR 2)

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -g -DGLM_ENABLE_EXPERIMENTAL")

include_directories(/usr/local/include)
link_directories(/usr/local/lib)

# Main renderer library
add_library(render STATIC Rasteriser.cpp Framebuffer.cpp Depthbuffer.cpp Viewport.cpp Shader.cpp ShadingGeometry.cpp RenderPrimitive.cpp Camera.cpp)

# Geometry library
add_library(geometry STATIC Geometry2.cpp CubeGeometry.cpp RandomTriangleGeometry.cpp GridGeometry.cpp PlyGeometry.cpp)

add_executable(srender main.cpp)
target_link_libraries(SRenderer render geometry)
target_link_libraries(SRenderer GLEW GL GLU glut)
