cmake_minimum_required(VERSION 3.9.0)

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -g -DGLM_ENABLE_EXPERIMENTAL")

include(FetchContent)
FetchContent_Declare(
gunit
GIT_REPOSITORY https://github.com/cpp-testing/GUnit.git
GIT_TAG        master
)
FETCHCONTENT_MAKEAVAILABLE(gunit)

include_directories(../include)

include_directories(../include)
add_executable(clipper-test ClipperTest.cpp)
target_link_libraries(clipper-test gfx1993 gunit)
add_test(NAME clipper COMMAND clipper-test)

add_executable(frustum-test FrustumTest.cpp)
target_link_libraries(frustum-test gfx1993 gunit)
add_test(NAME frustum COMMAND frustum-test)

add_executable(pipeline-test PipelineTest.cpp)
target_link_libraries(pipeline-test gfx1993 gunit)
add_test(NAME pipeline COMMAND pipeline-test)

add_executable(rasterizer-test RasterizerTest.cpp)
target_link_libraries(rasterizer-test gfx1993 gunit)
add_test(NAME rasterizer COMMAND rasterizer-test)

add_executable(renderconfig-test RenderConfigTest.cpp)
target_link_libraries(renderconfig-test gfx1993 gunit)
add_test(NAME renderconfig COMMAND renderconfig-test)

add_executable(texture-test TextureTest.cpp)
target_link_libraries(texture-test gfx1993 gunit)
add_test(NAME texture COMMAND texture-test)