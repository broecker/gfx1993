cmake_minimum_required(VERSION 3.9.0)
project(gfx1993-demos)


# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_ENABLE_EXPERIMENTAL -O3 -fopenmp")

cmake_policy(SET CMP0015 NEW)

# Local dependencies.
include_directories(../include)

include(FetchContent)

# SDL2 is looked up on the system first; if it isn't installed, it's fetched
# and built from source instead. glm is already resolved by the top-level
# CMakeLists.txt (as glm::glm), which runs before this subdirectory.
find_package(SDL2 CONFIG QUIET)
if(NOT SDL2_FOUND)
    message(STATUS "SDL2 not found on the system, fetching it instead")
    set(SDL_TEST OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        SDL2
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-2.30.0
    )
    FetchContent_MakeAvailable(SDL2)
endif()

# Dear ImGui, used by the demo app harness to overlay FPS and rasterizer
# stats. Built from source since upstream ships no CMake package; only the
# core widgets plus the SDL2 + SDL_Renderer backends are needed.
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.92.9b
)
FetchContent_MakeAvailable(imgui)

add_library(imgui STATIC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl2.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)
target_include_directories(imgui PUBLIC
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)
target_link_libraries(imgui PUBLIC SDL2::SDL2)

# Library and demo app harness
add_library(gfx93-demo-app STATIC DemoApp.cpp DemoUtils.cpp StarField.cpp)
target_link_libraries(gfx93-demo-app PUBLIC gfx1993 PRIVATE SDL2::SDL2 glm::glm imgui)

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
add_executable(11-dynamic-sky 11-dynamic-sky.cpp)
target_link_libraries(11-dynamic-sky gfx93-demo-app)

# Frustum / BVH interactions
add_executable(12-bvh 12-bvh.cpp)
target_link_libraries(12-bvh gfx93-demo-app)

# Frustum / AABB culling
add_executable(13-frustum-aabb 13-frustum-aabb.cpp)
target_link_libraries(13-frustum-aabb gfx93-demo-app)

# Headless performance benchmark (True headless - no SDL dependency)
add_executable(headless_benchmark headless_benchmark.cpp)
target_link_libraries(headless_benchmark gfx1993 glm::glm)