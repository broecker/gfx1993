#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "geometry/Quad.h"
#include "rendering/Pipeline.h"
#include "rendering/Shader.h"
#include "rendering/Texture.h"
#include "GlutDemoApp.h"

class TextureShader : public render::FragmentShader {
public:
    TextureShader(std::unique_ptr<render::Texture>&& texture) : texture(std::move(texture)) {
    }

    render::Fragment &&shadeSingle(const render::ShadingGeometry &in) override
    {
        render::Fragment fragment;
        fragment.color = texture->getTexel(in.texcoord);
        return std::move(fragment);
    }

private:
    std::unique_ptr<render::Texture> texture;
};

class Demo06 : public GlutDemoApp
{
public:
    Demo06() : GlutDemoApp("Demo 06 - Textures", 640, 480) {}

protected:
    void init() override
    {

        auto quadTexture = std::make_unique<render::Texture>(64, 64, glm::vec4(5.f, 0.f, 5.f, 1.f));
        quadTexture->makeCheckerboard(glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec4(1.f, 1.f, 0.f, 1.f), 8);
        //quadTexture->loadPPM("./colors.ppm");

        textureShader = std::make_shared<TextureShader>(std::move(quadTexture));

        shaders.vertexShader = std::make_shared<render::DefaultVertexTransform>();
        shaders.fragmentShader = textureShader;

        rasterizer->setRenderOutput(renderTarget);
        rasterizer->setShaders(shaders);

        quad = std::make_unique<geometry::Quad>(glm::vec4(1));

        // Put the quad on the floor.
        quad->transform = glm::rotate(glm::radians(90.f), glm::vec3(1.f,0.f,0.f));
        // And scale it up by 5
        quad->transform *= glm::scale(glm::vec3(5));
    }

    void updateFrame(float dt) override
    {
    }

    void renderFrame() override
    {
        // Clear the buffers
        renderTarget.framebuffer->clear(glm::vec4(0.7f, 0.7f, 0.9f, 1));
        renderTarget.depthbuffer->clear();

        // reset the render matrices
        render::DefaultVertexTransform *dvt = dynamic_cast<render::DefaultVertexTransform *>(shaders.vertexShader.get());

        dvt->modelMatrix = glm::mat4(1.f);
        dvt->viewMatrix = camera->getViewMatrix();
        dvt->projectionMatrix = camera->getProjectionMatrix();

        try {
            dvt->modelMatrix = quad->transform;
            rasterizer->drawTriangles(quad->getVertices(), quad->getIndices());
        }
        catch (const char *txt) {
            std::cerr << "Render error :\"" << txt << "\"\n";
        }
    }

private:
    std::unique_ptr<geometry::Quad> quad;
    std::shared_ptr<TextureShader> textureShader;
};


int main(int argc, char **argv)
{
    Demo06 demo;
    demo.run(argc, argv);

    return 0;
}
