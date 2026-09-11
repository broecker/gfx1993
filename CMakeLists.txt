cmake_minimum_required(VERSION 3.12.0)
project(gfx1993 VERSION 0.5.0 LANGUAGES CXX)

set(DEFAULT_BUILD_TYPE "Debug")  

option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O2 -DGLM_ENABLE_EXPERIMENTAL")
set(CMAKE_CXX_STANDARD 14)

# Requirements

find_package(glm REQUIRED)
find_package(absl CONFIG REQUIRED)

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
    src/RenderProfileInfo.cpp
    src/Shader.cpp
    src/Texture.cpp
    src/Viewport.cpp
)

target_include_directories(gfx1993 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
    PRIVATE src)

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

# --- Benchmark Automation & Dependency Verification ---

# The comparison script requires Python 3 and Git.
find_package(Python3 COMPONENTS Interpreter)
find_program(GIT_EXECUTABLE NAMES git)

# Optional: Check for memory profiling dependencies (tcmalloc and pprof)
find_library(TCMALLOC_LIB NAMES tcmalloc)
find_program(PPROF_EXECUTABLE NAMES pprof)
find_program(XVFB_RUN NAMES xvfb-run)

if(Python3_Interpreter_FOUND AND GIT_EXECUTABLE)
    # Standard performance comparison
    add_custom_target(compare-benchmarks
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/compare_benchmarks.py"
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Running automated benchmark comparison (Current vs HEAD~1)"
        USES_TERMINAL
    )

    # Memory profiling comparison
    add_custom_target(compare-benchmarks-pprof
        COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/scripts/compare_benchmarks.py" --pprof
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Running automated benchmark with memory profiling (requires tcmalloc and pprof)"
        USES_TERMINAL
    )

    if(NOT TCMALLOC_LIB OR NOT PPROF_EXECUTABLE)
        message(STATUS "Note: tcmalloc or pprof not found. Memory profiling targets may have limited functionality.")
    endif()

    if(NOT XVFB_RUN)
        message(STATUS "Note: xvfb-run not found. Interactive demos cannot run headless, but headless_benchmark will work fine.")
    endif()
else()
    message(STATUS "Python3_EXECUTABLE: ${Python3_EXECUTABLE}")
    message(STATUS "GIT_EXECUTABLE: ${GIT_EXECUTABLE}")
    message(WARNING "Required tools (Python3 or Git) missing. Automated benchmark targets were NOT created.")
endif()