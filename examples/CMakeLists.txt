cmake_minimum_required(VERSION 3.9.0)
project(gfx1993-demos)


# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_ENABLE_EXPERIMENTAL -O3 -fopenmp")

cmake_policy(SET CMP0015 NEW)

# Local dependencies.
include_directories(../include)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
find_package(glm REQUIRED)

# Library and demo app harness
add_library(gfx93-demo-app STATIC DemoApp.cpp DemoUtils.cpp)
target_link_libraries(gfx93-demo-app PUBLIC gfx1993 PRIVATE ${SDL2_LIBRARIES} glm::glm)

# A simple rotating triangle
add_executable(00-triangle 00-triangle.cpp)
target_link_libraries(00-triangle gfx93-demo-app)

# A simple rotating triangle, this time without OpenGL
add_executable(01-headless 01-headless.cpp)
target_link_libraries(01-headless gfx1993 glm::glm)

# Basic camera controls and geometry loading
add_executable(02-geometry 02-geometry.cpp)
target_link_libraries(02-geometry gfx93-demo-app)

# Line strips and creating custom geometry
add_executable(03-linestrips 03-linestrips.cpp)
target_link_libraries(03-linestrips gfx93-demo-app)

# Transparency and depth sorting
add_executable(04-transparency 04-transparency.cpp)
target_link_libraries(04-transparency gfx93-demo-app)

# Line and triangle clipping
add_executable(05-clipping 05-clipping.cpp)
target_link_libraries(05-clipping gfx93-demo-app)

# Texture mapping
add_executable(06-textures 06-textures.cpp)
target_link_libraries(06-textures gfx93-demo-app)

# View frustum and culling
add_executable(07-frustum 07-frustum.cpp)
target_link_libraries(07-frustum gfx93-demo-app)

# Wireframe and hidden-line culling
add_executable(08-wireframe 08-wireframe.cpp)
target_link_libraries(08-wireframe gfx93-demo-app)

# Different shading modes.
add_executable(09-shading 09-shading.cpp)
target_link_libraries(09-shading gfx93-demo-app)

# Skybox rendering.
add_executable(10-skybox 10-skybox.cpp)
target_link_libraries(10-skybox gfx93-demo-app)

# Physically based sky, baked into a lat/long texture on a background thread.
add_executable(10b-hillaire-skybox 10b-hillaire-skybox.cpp)
target_link_libraries(10b-hillaire-skybox gfx93-demo-app)

# Frustum / BVH interactions
add_executable(11-bvh 11-bvh.cpp)
target_link_libraries(11-bvh gfx93-demo-app)

# Frustum / AABB culling
add_executable(12-frustum-aabb 12-frustum-aabb.cpp)
target_link_libraries(12-frustum-aabb gfx93-demo-app)

# Headless performance benchmark (True headless - no SDL dependency)
add_executable(headless_benchmark headless_benchmark.cpp)
target_link_libraries(headless_benchmark gfx1993 glm::glm)