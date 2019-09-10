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

# The full monty test
add_executable(srender main.cpp)
target_link_libraries(srender common geometry rendering)
target_link_libraries(srender GLEW GL GLU glut)

# A simple rotating triangle
add_executable(00-triangle 00-triangle.cpp)
target_link_libraries(00-triangle common geometry rendering)
target_link_libraries(00-triangle GLEW GL GLU glut)

# A simple rotating triangle, this time without OpenGL
add_executable(01-headless 01-headless.cpp)
target_link_libraries(01-headless common geometry rendering)
