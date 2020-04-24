#include <iostream>
#include <memory>

#include <glm/glm.hpp>
#include <glm/gtx/polar_coordinates.hpp>

#include "rendering/Pipeline.h"
#include "rendering/Shader.h"
#include "common/Camera.h"
#include "geometry/GridGeometry.h"
#include "GlutDemoApp.h"

class WireSphere : public geometry::Geometry
{
public:
    WireSphere(float radius)
    {
        // Circles of latitude (N - S), excluding the poles.
        for (int theta = -80; theta < 90; theta += 10) {
            // A full circle on the horizontal plane.
            for (int phi = 0; phi <= 360; phi += 10) {
                glm::vec3 pos = glm::euclidean(glm::radians(glm::vec2(theta, phi))) * radius;
                glm::vec3 normal = glm::normalize(pos);
                glm::vec2 texcoord = glm::vec2(phi, theta) / glm::vec2(90, 360);
                glm::vec4 color = glm::vec4(1);

                render::Vertex v(glm::vec4(pos, 1), normal, color, texcoord);
                vertices.push_back(v);
                indices.push_back(vertices.size() - 1);
            }
        }

        // Circles of longitude (E-W).
        for (int phi = 0; phi <= 360; phi += 10) {
            // A full circle on the horizontal plane.
            for (int theta = -80; theta < 90; theta += 10) {
                glm::vec3 pos = glm::euclidean(glm::radians(glm::vec2(theta, phi))) * radius;
                glm::vec3 normal = glm::normalize(pos);
                glm::vec2 texcoord = glm::vec2(phi, theta) / glm::vec2(90, 360);
                glm::vec4 color = glm::vec4(1);

                render::Vertex v(glm::vec4(pos, 1), normal, color, texcoord);
                vertices.push_back(v);
                indices.push_back(vertices.size() - 1);
            }
        }

        // Note, this creates a giant 'tunnel' through the sphere's center. Ideally, we want to draw all the line
        // loops in multiple draw calls.
    }
};

class Demo03 : public GlutDemoApp
{
public:
    Demo03() : GlutDemoApp("Demo 03 - Line strips", 640, 480) {}

protected:
    void init() override
    {
        shaders.vertexShader = std::make_shared<render::DefaultVertexTransform>();
        shaders.fragmentShader = std::make_shared<render::SingleColorShader>(glm::vec4(0, 0, 1, 1));;

        rasterizer->setRenderOutput(renderTarget);
        rasterizer->setShaders(shaders);

        sphere = std::make_unique<WireSphere>(10.0f);
        camera = std::make_unique<OrbitCamera>(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 10.0f);
    }

    void updateFrame(float dt) override
    {}

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
            rasterizer->drawLineStrip(sphere->getVertices(), sphere->getIndices());
        }
        catch (const char *txt) {
            std::cerr << "Render error :\"" << txt << "\"\n";
        }
    }

    void handleKeyboard(unsigned char key, int x, int y)
    {
        if (key == 'b')
            rasterizer->toggleBoundingBoxes();
    }

private:
    std::unique_ptr<WireSphere> sphere;

};

int main(int argc, char **argv)
{
    Demo03 demo;
    demo.run(argc, argv);
    return 0;
}
