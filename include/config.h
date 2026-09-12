#ifndef GFX1993_BASE_CONFIG_INCLUDED
#define GFX1993_BASE_CONFIG_INCLUDED

// Enables or disables Tracy profiling zones in the rasterizer.
// Profiling carries a small cost. Actual capture only happens when
// the app is built with Tracy's client enabled (GFX1993_ENABLE_TRACY
// CMake option) and a Tracy profiler GUI is connected.
#define GFX1993_ENABLE_PROFILING 1

// If set, prints error messages to stdout on invalid configurations.
#define GFX1993_VERBOSE_RENDER_CONFIG 1

// TODO: add base types, such as Framebuffer and Depthbuffer or
// Indexlist data types.


// How many varying vec4s are supported.
#define GFX1993_SHADER_VARYING_COUNT 4

// If set, uses OpenMP parallel loops to resample and copy the target render
// buffer to the window. This is only useful in the default demo app. When using
// hardware textures or surfaces to display the result (for example, in OpenGL
// or Vulkan) this can be left off (I think).
#define GFX1993_DEMO_USE_OPENMP 1

// Rasterizer::threadPool is the single gfx1993::ThreadPool shared by every
// parallel stage below plus triangle-vs-plane clipping
// (Clipper::clipTrianglesToNdc) -- one set of worker threads for the whole
// rasterizer, instead of each stage spinning up its own. Points and lines
// are still processed serially; testing showed no performance increase.

// If set, uses the shared thread pool to transform vertices into clip space;
// i.e. it runs the Vertex Shader in parallel.
#define GFX1993_PARALLEL_TRANSFORM 1

// If set, uses the shared thread pool to shade the screen-filling quad.
#define GFX1993_PARALLEL_SHADE_SCREENQUAD 1

// If set, uses the shared thread pool to shade screen triangles. This is
// only useful for large (in screen-space coverage) triangles. For smaller
// ones, the additional setup cost outweighs the performance gains.
#define GFX1993_PARALLEL_SHADE_TRIANGLE 0

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
