#ifndef GFX1993_RENDERCONFIG_H
#define GFX1993_RENDERCONFIG_H

#include <memory>

#include <glm/glm.hpp>

#include "Depthbuffer.h"
#include "Framebuffer.h"

namespace render
{

    class VertexShader;
    class FragmentShader;

    class Viewport;

    struct RenderConfig
    {
        std::shared_ptr<Framebuffer>    framebuffer;
        std::shared_ptr<Depthbuffer>    depthbuffer;

        std::shared_ptr<Viewport>       viewport;

        std::shared_ptr<VertexShader>   vertexShader;
        std::shared_ptr<FragmentShader> fragmentShader;

        // debug flags
        bool drawTriangleBounds = false;


        void clearBuffers(const glm::vec4& clearColor);

        inline bool hasValidRenderOutput() const
        {
            return viewport && (framebuffer || depthbuffer);
        }

        inline bool hasValidShaderConfiguration() const
        {
            return vertexShader && fragmentShader;
        }

        inline bool isValid() const
        {
            return hasValidRenderOutput() && hasValidShaderConfiguration();
        }



    };

}

#endif //GFX1993_RENDERCONFIG_H
