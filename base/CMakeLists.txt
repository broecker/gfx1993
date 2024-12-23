cmake_minimum_required(VERSION 3.5.0)

# Compile flag to enable experimental glm features.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_ENABLE_EXPERIMENTAL")

# Geometry library
add_library(gfx93-base STATIC config.h Pipeline.h Pipeline.cpp)

include(FetchContent)
FetchContent_Declare(
gunit
GIT_REPOSITORY https://github.com/cpp-testing/GUnit.git
GIT_TAG        master
)
FETCHCONTENT_MAKEAVAILABLE(gunit)

add_executable(gfx93-pipeline-test PipelineTest.cpp)
target_link_libraries(gfx93-pipeline-test gfx93-base gunit)
