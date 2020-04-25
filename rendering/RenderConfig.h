#ifndef GFX1993_RENDERCONFIG_H
#define GFX1993_RENDERCONFIG_H

#include <memory>

#include <glm/glm.hpp>

#include "Depthbuffer.h"
#include "Framebuffer.h"

namespace render {

class VertexShader;
class FragmentShader;

class Viewport;

// Describes the configuration, input/output and options that will be used to
// render primitives. This is passed in explicitly to the various rasterization
// methods.
struct RenderConfig {
  // The output buffers. At least either a framebuffer or a depthbuffer must be
  // set. If both are set, they must have the same dimensions.
  std::shared_ptr<Framebuffer> framebuffer;
  std::shared_ptr<Depthbuffer> depthbuffer;

  // The viewport within the buffers we're rendering to.
  std::shared_ptr<Viewport> viewport;

  // The vertex and framgent shader that will be used to render primitives.
  std::shared_ptr<VertexShader> vertexShader;
  std::shared_ptr<FragmentShader> fragmentShader;

  // Rasterizer behavior
  // Enable/disable alpha blending. When enabled, writes to the fragment buffer
  // will be mixed with already written values. This can be expensive, so it is
  // optional. The blend functions is: (existing color * 1-alpha) + (new color *
  // alpha) which corresponds to the GL_ALPHA, GL_ONE_MINUS_ALPHA blend
  // function.
  bool alphaBlending = false;

  // Debug flags follow.

  // If set to true, bounding areas will be drawn around rasterized triangles.
  bool drawTriangleBounds = false;

  // Utility method to clear both frame and depth buffer with a single call.
  void clearBuffers(const glm::vec4 &clearColor);

  // Checks that we have at least a single render target and a viewport and that
  // the dimensions match.
  bool hasValidRenderOutput() const;

  // We need both a vertex and frament shader for rendering.
  inline bool hasValidShaderConfiguration() const {
    return vertexShader && fragmentShader;
  }

  inline bool isValid() const {
    return hasValidRenderOutput() && hasValidShaderConfiguration();
  }
};

} // namespace render

#endif // GFX1993_RENDERCONFIG_H
