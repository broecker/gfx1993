cmake_minimum_required(VERSION 3.5.0)
enable_testing()

# Main renderer library
add_library(gfx93-rendering STATIC Rasterizer.cpp Viewport.cpp Shader.cpp Clipper.cpp Clipper.h Texture.h Texture.cpp RenderConfig.h RenderConfig.cpp Framebuffer.h Framebuffer.cpp Depthbuffer.h Depthbuffer.cpp)
target_link_libraries(gfx93-rendering gfx93-base)
target_link_libraries(gfx93-rendering gfx93-util)

set_property(TARGET gfx93-rendering PROPERTY CXX_STANDARD 17)

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -g -DGLM_ENABLE_EXPERIMENTAL")

include(FetchContent)
FetchContent_Declare(
gunit
GIT_REPOSITORY https://github.com/cpp-testing/GUnit.git
GIT_TAG        master
)
FETCHCONTENT_MAKEAVAILABLE(gunit)

add_executable(gfx93-rendering-clipper-test ClipperTest.cpp)
target_link_libraries(gfx93-rendering-clipper-test gfx93-rendering gunit)