cmake_minimum_required(VERSION 3.12.0)
project(gfx1993 VERSION 0.5.0 LANGUAGES CXX)

set(DEFAULT_BUILD_TYPE "Debug")  

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -DGLM_ENABLE_EXPERIMENTAL")
set(CMAKE_CXX_STANDARD 17)

# Requirements

find_package(glm REQUIRED)
find_package(absl CONFIG REQUIRED)

# Tracy profiler client. Zones are instrumented via include/Profiler.h;
# actual capture only happens once a Tracy profiler GUI connects.
option(GFX1993_ENABLE_TRACY "Build with the Tracy profiler client" ON)
if(GFX1993_ENABLE_TRACY)
    include(FetchContent)
    set(TRACY_ENABLE ON CACHE BOOL "" FORCE)
    FetchContent_Declare(
        tracy
        GIT_REPOSITORY https://github.com/wolfpld/tracy.git
        GIT_TAG        v0.11.1
    )
    FetchContent_MakeAvailable(tracy)
endif()

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

add_library(gfx1993
    src/BoundingVolumes.cpp
    src/Clipper.cpp
    src/Camera.cpp  
    src/Depthbuffer.cpp
    src/Framebuffer.cpp
    src/Frustum.cpp
    src/Geometry.cpp
    src/Pipeline.cpp
    src/PlyGeometry.cpp
    src/Rasterizer.cpp
    src/RenderConfig.cpp
    src/RenderDebugInfo.cpp
    src/Shader.cpp
    src/Texture.cpp
    src/Viewport.cpp
)

target_include_directories(gfx1993 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

if(GFX1993_ENABLE_TRACY)
    target_link_libraries(gfx1993 PUBLIC TracyClient)
endif()

include(GNUInstallDirs)
install(TARGETS ${PROJECT_NAME} EXPORT gfx1993-config  
    ARCHIVE  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY  DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME  DESTINATION ${CMAKE_INSTALL_BINDIR})


install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME})

install(EXPORT gfx1993-config DESTINATION share/gfx1993/cmake)

export(TARGETS ${PROJECT_NAME} FILE gfx1993-config.cmake)

add_subdirectory(examples)

enable_testing()
add_subdirectory(tests)