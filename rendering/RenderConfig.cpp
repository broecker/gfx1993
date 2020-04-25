#include "RenderConfig.h"

namespace render
{
    void RenderConfig::clearBuffers(const glm::vec4& clearColor)
    {
        if (framebuffer)
            framebuffer->clear(clearColor);
        if (depthbuffer)
            depthbuffer->clear();

    }


}
