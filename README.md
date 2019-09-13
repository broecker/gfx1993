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

## TODOs
This is an unsorted list of outstanding tasks.

- Clean up the rasterizer. There's a lot of old, untouched code in there. Rasterizer should become an abstract class so 
that specialized rasterizers can be easily implemented.
- Debug clipping. Both line and triangle clipping seem broken
- Write tests! Especially for framebuffer and rasterizer.
- Implement bounding geometries and view frustum culling
- Add FPS camera
- Add true 256 color rendering. We can override the Framebuffer to do so. Add a RGB -> indexed color translation table
and do a Voronoi triangulation on the input. Then, when plotting, pick the correct index color through the
triangulation.
- Add dithering
- Add textures and texture lookups
- Add png input/output
- Add sample shaders that read a framebuffer as a texture
- Enable selective color/depth writes
- Add example with BSP tree traversal after disabling depth writes.
- Add other rasterizers; for example a Span Renderer
- Implement tile-based rendering and parallel/async rasterization
- Add WAD file loading :) 
- Add Voxel and Voxelspace-like rendering
- Add example with textures and programmatic skybox rendering
- Add example about depth sorting
- Add profiler
- Add portal-based rendering demo
