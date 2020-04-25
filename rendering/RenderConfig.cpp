#include "RenderConfig.h"

namespace render {
void RenderConfig::clearBuffers(const glm::vec4 &clearColor) {
  if (framebuffer)
    framebuffer->clear(clearColor);
  if (depthbuffer)
    depthbuffer->clear();
}

bool RenderConfig::hasValidRenderOutput() const {
  // If both framebuffer and depth buffer are set, check that they have the same
  // dimensions.
  if (framebuffer && depthbuffer) {
    if ((framebuffer->getWidth() != depthbuffer->getWidth()) ||
        (framebuffer->getHeight() != depthbuffer->getHeight()))
      return false;
  }

  // Make sure we have a viewport and at least a single render target.
  return viewport && (framebuffer || depthbuffer);
}
} // namespace render
