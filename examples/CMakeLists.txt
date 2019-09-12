cmake_minimum_required (VERSION 2.6)

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_ENABLE_EXPERIMENTAL")

include_directories(/usr/local/include)
include_directories(..)

link_directories(/usr/local/lib)
link_directories(../common)
link_directories(../geometry)
link_directories(../rendering)

# A simple rotating triangle
add_executable(00-triangle 00-triangle.cpp 03-linestrips.cpp)
target_link_libraries(00-triangle gfx93-common gfx93-geometry gfx93-rendering)
target_link_libraries(00-triangle GL GLU glut)

# A simple rotating triangle, this time without OpenGL
add_executable(01-headless 01-headless.cpp)
target_link_libraries(01-headless gfx93-common gfx93-geometry gfx93-rendering)

# Basic camera controls and geometry loading
add_executable(02-geometry 02-geometry.cpp)
target_link_libraries(02-geometry gfx93-common gfx93-geometry gfx93-rendering)
target_link_libraries(02-geometry GL GLU glut)

# Line strips and creating custom geometry
# Basic camera controls and geometry loading
add_executable(03-linestrips 03-linestrips.cpp)
target_link_libraries(03-linestrips gfx93-common gfx93-geometry gfx93-rendering)
target_link_libraries(03-linestrips GL GLU glut)