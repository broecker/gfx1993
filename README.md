# Gfx:1993 - Artisanal Software Rendering

## Wat
This is a software-based rasterizer/3D engine. It's fairly low-level and
barebones. I use it to explore and teach myself about 3D graphics pipelines.
When I started working on it, OpenGL 2.0 was widely used, so that's what I used
as a model for the pipeline. 

The rasterizer supports triangle, line and point primitives provided as vertex
and index lists. It uses a vertex and a fragment shader to actually transform
the vertices and calculate the fragment color, so it's fairly flexible.
Performance, however, is not the prime focus of this project. 

## License
MIT Open-Source license; see LICENSE in this root folder. 

## Build and Dependencies
The project's build file can be created using cmake. Tested on Ubuntu without
problems but YMMV. 

To build run:
```sh
cmake -B build -S . -DCMAKE_INSTALL_PREFIX:PATH=${HOME}
cd build
make -j

# Optionally:
make install
```

Unit tests can be found under `tests/` and run with `ctest`. Example binaries
are found in `examples/` and can be run individually.

Dependencies: 
- glm
- SDL2
- abseil

CMake looks for these on the system first; any that aren't installed are
fetched and built from source automatically instead, so a fresh checkout
builds without installing anything by hand. The
[Tracy](https://github.com/wolfpld/tracy) profiler client and
[Dear ImGui](https://github.com/ocornut/imgui) (used by the example demo app
for its FPS/rasterizer-stats overlay) are always fetched and built this way,
since neither ships an installable CMake package.

## TODOs
This is an unsorted list of outstanding tasks.

- Clean up the rasterizer. There's a lot of old, untouched code in there. Rasterizer should become an abstract class so 
that specialized rasterizers can be easily implemented.
- Add true 256 color rendering. We can override the Framebuffer to do so. Add an
RGB -> indexed color translation table
and do a Voronoi triangulation on the input. Then, when plotting, pick the
correct index color through triangulation.
- Add dithering
- Add png input/output
- Add example with BSP tree traversal after disabling depth writes.
- Add other rasterizers; for example a Span Renderer
- Implement tile-based rendering (rasterization is already parallelized across triangles, but not tiled)
- Add WAD file loading :) 
- Add Voxel and Voxelspace-like rendering
- Add portal-based rendering demo
