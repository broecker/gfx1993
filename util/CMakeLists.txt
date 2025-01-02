cmake_minimum_required(VERSION 3.5.0)

# Compile flag to enable experimental glm features.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_ENABLE_EXPERIMENTAL")

# Geometry library
add_library(gfx93-util STATIC Camera.cpp RenderDebugInfo.h RenderDebugInfo.cpp RenderProfileInfo.h RenderProfileInfo.cpp Frustum.h Frustum.cpp BoundingVolumes.h BoundingVolumes.cpp)


include(FetchContent)
FetchContent_Declare(
  gunit
  GIT_REPOSITORY https://github.com/cpp-testing/GUnit.git
  GIT_TAG        master
)
FETCHCONTENT_MAKEAVAILABLE(gunit)

add_executable(gfx93-util-frustum-test FrustumTest.cpp)
target_link_libraries(gfx93-util-frustum-test gfx93-util gunit)
