#include "RenderConfig.h"

#include <iostream>

namespace gfx1993 {
void RenderConfig::clearBuffers(const glm::vec4 &clearColor) {
  if (framebuffer)
    framebuffer->clear(clearColor);
  if (depthbuffer)
    depthbuffer->clear();
}

bool RenderConfig::hasValidShaderConfiguration() const {
  if (vertexShader == nullptr) {
#if GFX1993_VERBOSE_RENDER_CONFIG
    std::cout << "[RenderConfig] No vertex shader attached!\n";
#endif
    return false;
  }

  if (fragmentShader == nullptr) {
#if GFX1993_VERBOSE_RENDER_CONFIG
    std::cout << "[RenderConfig] No fragment shader attached!\n";
#endif
    return false;
  }

  return vertexShader && fragmentShader;
}

bool RenderConfig::hasValidRenderOutput() const {
  // If both framebuffer and depth buffer are set, check that they have the same
  // dimensions.

  // Make sure we have a viewport and at least a single render target.
  if (framebuffer == nullptr && depthbuffer == nullptr) {
#if GFX1993_VERBOSE_RENDER_CONFIG
    std::cout << "[RenderConfig] Error: no Framebuffer or Depthbuffer attached.\n";
#endif
    return false;
  }

  if (!viewport) {
#if GFX1993_VERBOSE_RENDER_CONFIG
    std::cout << "[RenderConfig] Error: No viewport attached.\n";
#endif
    return false;
  }

  if (framebuffer && depthbuffer) {
    if ((framebuffer->getWidth() != depthbuffer->getWidth()) ||
        (framebuffer->getHeight() != depthbuffer->getHeight())) {
#if GFX1993_VERBOSE_RENDER_CONFIG
      std::cout << "[RenderConfig] Error: " <<
        "Framebuffer [" << framebuffer->getWidth() << "x" << framebuffer->getHeight() << "] and " <<
        "Depthbuffer [" << depthbuffer->getWidth() << "x" << depthbuffer->getHeight() << "] have differing resolutions\n.";
#endif
      return false;
    }
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
#if GFX1993_VERBOSE_RENDER_CONFIG
  if (!oddPointSize) {
    std::cout << "[RenderConfig] pointSize must be odd!\n";
  }
#endif


  return hasValidRenderOutput() && hasValidShaderConfiguration() && oddPointSize;
}

} // namespace gfx1993