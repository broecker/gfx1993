#include <iostream>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "GlutDemoApp.h"
#include "rendering/Pipeline.h"
#include "rendering/Shader.h"

class Demo00 : public GlutDemoApp
{
public:
    Demo00() : GlutDemoApp("Demo 00 - Hello Triangle"), rotationAngle(0) {}

protected:
    void init() override {
        using render::Vertex;

        // Keep a separate reference to the vertex shader so we can change the transform easily.
        vertexShader = std::make_shared<render::DefaultVertexTransform>();
        renderConfig.vertexShader = vertexShader;
        renderConfig.fragmentShader = std::make_shared<render::InputColorShader>();

        // Create the triangle geometry
        vertices.push_back(Vertex(glm::vec4(-5, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(0, 0, 1, 1), glm::vec2(0, 0)));
        vertices.push_back(Vertex(glm::vec4(0, 5, 0, 1), glm::vec3(1, 0, 0), glm::vec4(1, 0, 0, 1), glm::vec2(0, 0)));
        vertices.push_back(Vertex(glm::vec4(5, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(0, 1, 0, 1), glm::vec2(0, 0)));

        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(0);
        indices.push_back(1);
    }

    void updateFrame(float dt) override
    {
        rotationAngle += dt * 3.f;
    }

    void renderFrame() override
    {
        // clear the buffers
        renderConfig.clearBuffers(glm::vec4(0, 0, 0.2f, 1));

        // reset the render matrices
        vertexShader->modelMatrix = glm::rotate(rotationAngle, glm::vec3(0, 1, 0));
        vertexShader->viewMatrix = glm::lookAt(glm::vec3(0, 1, -10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
        vertexShader->projectionMatrix = glm::perspective(glm::radians(60.f), (float) width / height, 0.2f, 100.f);

        try {
            rasterizer->drawTriangles(renderConfig, vertices, indices);
        }
        catch (const char *txt) {
            std::cerr << "Render error :\"" << txt << "\"\n";
        }
    }

private:
    float rotationAngle;

    render::VertexList vertices;
    render::IndexList indices;

    std::shared_ptr<render::DefaultVertexTransform> vertexShader;
};

int main(int argc, char **argv)
{
    Demo00 demo;
    demo.run(argc, argv);

    return 0;
}
