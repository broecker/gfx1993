# Gfx:1993 - Artisanal Software Rendering

## Wat
This is a software-based rasterizer/3D engine. It's fairly low-level and barebones. I use it to explore and teach myself
about 3D graphics pipelines. When I started working on it, OpenGL 2.0 was widely used, so that's what I used as a model
for the pipeline. 

The rasterizer supports triangle, line and point primitives provided as vertex and index lists. It uses a vertex and a
fragment shader to actually transform the vertices and calculate the fragment color, so it's fairly flexible.
Performance, however, is not the prime focus of this project. 

## License
MIT Open-Source license; see LICENSE in this root folder. 

## Build and Dependencies
The project's build file can be created using cmake. Tested on Ubuntu without problems but YMMV. 

Dependencies: 
- libglm
- glut/freeglut
- GLU and and OpenGL