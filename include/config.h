#ifndef GFX1993_BASE_CONFIG_INCLUDED
#define GFX1993_BASE_CONFIG_INCLUDED

// Enables or disables walltime-profiling in the the rasterizer. 
// Profiling carries a small cost.
#define GFX1993_ENABLE_PROFILING 1
// Debug profiling will print the name of the rasterizer stage
// during execution. This will slow things down a lot but can be
// very helpful during debugging. 
#define GFX1993_ENABLE_DEBUG_PROFILING 0

// TODO: add base types, such as Framebuffer and Depthbuffer or
// Indexlist data types.


// How many varying vec4s are supported.
#define GFX1993_SHADER_VARYING_COUNT 4

// If set, uses OpenMP parallel loops to resample and copy the target render
// buffer to the window. This is only useful in the default demo app. When using
// hardware textures or surfaces to display the result (for example, in OpenGL
// or Vulkan) this can be left off (I think).
#define GFX1993_DEMO_USE_OPENMP 1

// If set, uses OpenMP parallel loops to transform vertices into clip space;
// i.e. it runs the Vertex Shader in parallel.
#define GFX1993_PARALLEL_TRANSFORM 1

// If set, uses OpenMP parallel loops to clip triangles. Points and lines are
// still processed serially; testing showed no performance increase.
#define GFX1993_PARALLEL_CLIP 1

#define GFX1993_PARALLEL_SHADE_SCREENQUAD 1

// If set, uses OpenMP parallel lops to shade screen triangles. This is only
// useful for large (in screen-space coverage) triangles. For smaller ones,
// the additional setup cost outweighs the performance gains.
#define GFX1993_PARALLEL_SHADE_TRIANGLE 1

// If enabled, adds another layer to the depth buffer that measures writes to
// each pixel.
#define GFX1993_DEPTHBUFFER_LOG_WRITES 1

// Constants to follow.
namespace gfx1993 {

// Internal rendering resolution. Currently only used to set up the Demo App.
constexpr unsigned int VGA_WIDTH = 640;
constexpr unsigned int VGA_HEIGHT = 480;

}  // namespace gfx1993

#endif // GFX1993_BASE_CONFIG_INCLUDED
