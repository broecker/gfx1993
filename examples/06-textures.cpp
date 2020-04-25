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

class TextureShader : public render::FragmentShader
{
public:
    TextureShader() : texture(nullptr)
    {
    }

    render::Fragment &&shadeSingle(const render::ShadingGeometry &in) override
    {
        render::Fragment fragment;

        if (texture) {
            fragment.color = texture->getTexel(in.texcoord);
        }
        return std::move(fragment);
    }

    inline void setTexture(std::shared_ptr<render::Texture> texture) { this->texture = texture; }

private:
    std::shared_ptr<render::Texture> texture;
};

class TexCoordShader : public render::FragmentShader
{
public:
    render::Fragment &&shadeSingle(const render::ShadingGeometry &in) override
    {
        render::Fragment fragment;
        fragment.color = glm::vec4(in.texcoord, 0.f, 1.f);
        return std::move(fragment);
    }
};

class Demo06 : public GlutDemoApp
{
public:
    Demo06() : GlutDemoApp("Demo 06 - Textures") {}

protected:
    void init() override
    {
        textureShader = std::make_shared<TextureShader>();
        texCoordShader = std::make_shared<TexCoordShader>();

        renderConfig.vertexShader = std::make_shared<render::DefaultVertexTransform>();
        renderConfig.fragmentShader = texCoordShader;

        quad = std::make_unique<geometry::Quad>(glm::vec4(1));

        // Put the quad on the floor.
        quad->transform = glm::rotate(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
        // And scale it up by 5
        quad->transform *= glm::scale(glm::vec3(5));
    }

    void updateFrame(float dt) override
    {
    }

    void renderFrame() override
    {
        // Clear the buffers
        renderConfig.clearBuffers(glm::vec4(0.7f, 0.7f, 0.9f, 1));

        // reset the render matrices
        render::DefaultVertexTransform *dvt = dynamic_cast<render::DefaultVertexTransform *>(renderConfig.vertexShader.get());

        dvt->modelMatrix = glm::mat4(1.f);
        dvt->viewMatrix = camera->getViewMatrix();
        dvt->projectionMatrix = camera->getProjectionMatrix();

        try {
            dvt->modelMatrix = quad->transform;
            rasterizer->drawTriangles(renderConfig, quad->getVertices(), quad->getIndices());
        }
        catch (const char *txt) {
            std::cerr << "Render error :\"" << txt << "\"\n";
        }
    }

    void handleKeyboard(unsigned char key, int x, int y)
    {
        if (key == 'c') {
            textureShader->setTexture(render::Texture::makeCheckerboard(64, 64, 8, glm::vec4(1.f, 0.f, 0.f, 1.f),
                                                                        glm::vec4(1.f, 1.f, 0.f, 1.f)));
            renderConfig.fragmentShader = textureShader;
        }

        if (key == 'f') {
            textureShader->setTexture(render::Texture::loadPPM("./colors.ppm"));
            renderConfig.fragmentShader = textureShader;
        }

        if (key == 'l') {
            textureShader->setTexture(render::Texture::loadPPM("./lenna.ppm"));
            renderConfig.fragmentShader = textureShader;
        }

        // Set texture coord debugging shader.
        if (key == 't') {
            renderConfig.fragmentShader = texCoordShader;
        }
    }


private:
    std::unique_ptr<geometry::Quad> quad;
    std::shared_ptr<TextureShader> textureShader;
    std::shared_ptr<TexCoordShader> texCoordShader;
};


int main(int argc, char **argv)
{
    Demo06 demo;
    demo.run(argc, argv);

    return 0;
}
