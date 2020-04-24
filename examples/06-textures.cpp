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
    TextureShader() : texture(nullptr) {
    }

    render::Fragment &&shadeSingle(const render::ShadingGeometry &in) override
    {
        render::Fragment fragment;

        if (texture) {
            fragment.color = texture->getTexel(in.texcoord);
        }
        return std::move(fragment);
    }

    inline void setTexture(std::shared_ptr<render::Texture> texture) {this->texture = texture;}

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
    Demo06() : GlutDemoApp("Demo 06 - Textures", 640, 480) {}

protected:
    void init() override
    {
        textureShader = std::make_shared<TextureShader>();
        auto texture = std::make_shared<render::Texture>(64, 64);
        texture->makeCheckerboard(glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec4(1.f, 1.f, 0.f, 1.f), 8);
        textureShader->setTexture(texture);

        texCoordShader = std::make_shared<TexCoordShader>();

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

    void handleKeyboard(unsigned char key, int x, int y)
    {
        if (key == 'c')
        {
            auto texture = std::make_shared<render::Texture>(64, 64);
            texture->makeCheckerboard(glm::vec4(1.f, 0.f, 0.f, 1.f), glm::vec4(1.f, 1.f, 0.f, 1.f), 8);
            textureShader->setTexture(texture);
        }

        if (key == 'f')
        {
            auto texture = std::make_shared<render::Texture>(64, 64);
            texture->loadPPM("./colors.ppm");
            textureShader->setTexture(texture);
        }

        if (key == 'l')
        {
            auto texture = std::make_shared<render::Texture>(64, 64);
            texture->loadPPM("./lenna.ppm");
            textureShader->setTexture(texture);
        }

        // Toggle shaders
        if (key == 't')
        {
            if (shaders.fragmentShader == textureShader)
                shaders.fragmentShader = texCoordShader;
            else
                shaders.fragmentShader = textureShader;
            rasterizer->setShaders(shaders);
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