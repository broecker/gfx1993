cmake_minimum_required (VERSION 2.6)
enable_testing()

# Main renderer library
add_library(gfx93-rendering STATIC Rasterizer.cpp Framebuffer.cpp Depthbuffer.cpp Viewport.cpp Shader.cpp Pipeline.cpp Clipper.cpp Clipper.h)

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -g -DGLM_ENABLE_EXPERIMENTAL")

add_executable(gfx93-rendering-clipper-test ClipperTest.cpp)
target_link_libraries(gfx93-rendering-clipper-test gfx93-rendering)

add_test(PlaneWithFrontSpacePoint gfx93-rendering-clipper-test "plane-frontspace")
add_test(PlaneWithBackSpacePoint gfx93-rendering-clipper-test "plane-backspace")
add_test(PlaneWithCoplanarPoint gfx93-rendering-clipper-test "plane-coplanar")
add_test(PlaneSignedDistances gfx93-rendering-clipper-test "plane-distances")
add_test(ClipPointsOnSinglePlane gfx93-rendering-clipper-test "clipper-point-single-plane")
add_test(ClipPointsOnMultiplePlanes gfx93-rendering-clipper-test "clipper-point-multiple-planes")
add_test(ClipLineOnSinglePlaneKeepsInFront gfx93-rendering-clipper-test "clipper-line-single-plane-keep")
add_test(ClipLineOnSinglePlaneDiscardsBehind gfx93-rendering-clipper-test "clipper-line-single-plane-discard")
add_test(ClipLineOnSinglePlane gfx93-rendering-clipper-test "clipper-line-single-plane")
add_test(ClipLineOnMultiplePlanes gfx93-rendering-clipper-test "clipper-line-multiple-planes")
add_test(ClipLineOnCoplanarPlane gfx93-rendering-clipper-test "clipper-line-coplanar-plane")
add_test(ClipperCreatesNdcPlanes gfx93-rendering-clipper-test "clipper-creates-ndc-plane")
