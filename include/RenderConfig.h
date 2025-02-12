#ifndef GFX1993_RENDERCONFIG_H
#define GFX1993_RENDERCONFIG_H

#include <memory>

#include <glm/glm.hpp>

#include "Depthbuffer.h"
#include "Framebuffer.h"
#include "Viewport.h"

namespace gfx1993 {

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

  // If set to true, triangles facing away from our view will be discarded.
  bool cullBackFaces = true;

  // If set to true, we'll write to the depth buffer.
  bool depthWrite = true;
  // If set to true, we will only write to the depth and frame buffers if the
  // depth test succeeds. Disable to always write.
  bool depthTest = true;

  // The number of pixels a single point primitive should cover. This must be an
  // odd number.
  unsigned int pointSize = 1;

  // Debug flags follow.

  // If set to true, bounding areas will be drawn around rasterized triangles.
  bool drawTriangleBounds = false;

  // Utility method to clear both frame and depth buffer with a single call.
  void clearBuffers(const glm::vec4 &clearColor);

  // Checks that we have at least a single render target and a viewport and that
  // the dimensions match.
  bool hasValidRenderOutput() const;

  // We need both a vertex and frament shader for rendering.
  bool hasValidShaderConfiguration() const;

  bool isValid() const;
};

} // namespace gfx1993

#endif // GFX1993_RENDERCONFIG_H
