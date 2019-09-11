cmake_minimum_required (VERSION 2.6)
project (srender)

# The version number.
set (SRenderer_VERSION_MAJOR 0)
set (SRenderer_VERSION_MINOR 2)

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -O0 -g -DGLM_ENABLE_EXPERIMENTAL")

include_directories(/usr/local/include)

link_directories(/usr/local/lib)

# The libraries
add_subdirectory(common)
add_subdirectory(geometry)
add_subdirectory(rendering)

# The examples
add_subdirectory(examples)

# The full monty test
add_executable(srender main.cpp)
target_link_libraries(srender gfx93-common gfx93-geometry gfx93-rendering)
target_link_libraries(srender GL GLU glut)