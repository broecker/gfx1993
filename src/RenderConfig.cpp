#include "RenderConfig.h"

namespace gfx1993 {
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
  if (!viewport || !(framebuffer || depthbuffer)) {
    return false;
  }

  // Make sure the viewport fits the render target.
  if (framebuffer) {
    return (viewport->origin.x + viewport->size.x <= static_cast<int>(framebuffer->getWidth()) && 
            viewport->origin.y + viewport->size.y <= static_cast<int>(framebuffer->getHeight()));
  } else {
     return (viewport->origin.x + viewport->size.x <= static_cast<int>(depthbuffer->getWidth()) && 
            viewport->origin.y + viewport->size.y <= static_cast<int>(depthbuffer->getHeight()));   
  }
}

bool RenderConfig::isValid() const {
  bool oddPointSize = (pointSize % 2 ==1);
  return hasValidRenderOutput() && hasValidShaderConfiguration() && oddPointSize;
}


} // namespace render
} // namespace gfx1993