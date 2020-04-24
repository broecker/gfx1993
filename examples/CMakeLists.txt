cmake_minimum_required(VERSION 2.6)

# Compile + link setup
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGLM_ENABLE_EXPERIMENTAL")

cmake_policy(SET CMP0015 NEW)

include_directories(/usr/local/include)
include_directories(..)

link_directories(/usr/local/lib)
link_directories(../common)
link_directories(../geometry)
link_directories(../rendering)

# Library and demo app harness
add_library(gfx93-demo-app STATIC GlutDemoApp.cpp)
target_link_libraries(gfx93-demo-app gfx93-common gfx93-geometry gfx93-rendering)
target_link_libraries(gfx93-demo-app GL GLU glut)

# A simple rotating triangle
add_executable(00-triangle 00-triangle.cpp)
target_link_libraries(00-triangle gfx93-demo-app)

# A simple rotating triangle, this time without OpenGL
add_executable(01-headless 01-headless.cpp)
target_link_libraries(01-headless gfx93-common gfx93-geometry gfx93-rendering)

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

# Textures
add_executable(06-textures 06-textures.cpp)
target_link_libraries(06-textures gfx93-common gfx93-geometry gfx93-rendering)
target_link_libraries(06-textures GL GLU glut)