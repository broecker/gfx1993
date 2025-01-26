cmake_minimum_required(VERSION 3.9.0)
project(gfx1993 VERSION 0.5.0 LANGUAGES CXX)

set(DEFAULT_BUILD_TYPE "Debug")

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -DGLM_ENABLE_EXPERIMENTAL")
set(CMAKE_CXX_STANDARD 14)

# Requirements
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})
find_package(glm REQUIRED)
find_package(absl CONFIG REQUIRED)

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()


set(SOURCE_FILES
    src/BoundingVolumes.cpp
    src/Clipper.cpp
    src/Camera.cpp  
    src/CubeGeometry.cpp
    src/Depthbuffer.cpp
    src/Framebuffer.cpp
    src/Frustum.cpp
    src/Geometry.cpp
    src/GridGeometry.cpp
    src/Pipeline.cpp
    src/PlyGeometry.cpp
    src/Quad.cpp
    src/RandomTriangleGeometry.cpp
    src/Rasterizer.cpp
    src/RenderConfig.cpp
    src/RenderDebugInfo.cpp
    src/RenderProfileInfo.cpp
    src/Shader.cpp
    src/Sphere.cpp
    src/Teapot.cpp
    src/Terrain.cpp
    src/Texture.cpp
    src/Viewport.cpp
)
add_library(gfx1993
    src/BoundingVolumes.cpp
    src/Clipper.cpp
    src/Camera.cpp  
    src/CubeGeometry.cpp
    src/Depthbuffer.cpp
    src/Framebuffer.cpp
    src/Frustum.cpp
    src/Geometry.cpp
    src/GridGeometry.cpp
    src/Pipeline.cpp
    src/PlyGeometry.cpp
    src/Quad.cpp
    src/RandomTriangleGeometry.cpp
    src/Rasterizer.cpp
    src/RenderConfig.cpp
    src/RenderDebugInfo.cpp
    src/RenderProfileInfo.cpp
    src/Shader.cpp
    src/Sphere.cpp
    src/Teapot.cpp
    src/Terrain.cpp
    src/Texture.cpp
    src/Viewport.cpp
)

target_include_directories(gfx1993 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} EXPORT Gfx1993Config  
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})


install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

install(EXPORT Gfx1993Config DESTINATION share/gfx1993/cmake)

export(TARGETS ${PROJECT_NAME} FILE Gfx1993Config.cmake)

enable_testing()
add_subdirectory(tests)