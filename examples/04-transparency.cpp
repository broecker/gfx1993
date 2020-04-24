#include <cstdlib>
#include <iostream>
#include <memory>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/polar_coordinates.hpp>
#include <glm/gtx/io.hpp>

#include <GL/glut.h>
#include <rendering/Pipeline.h>
#include <geometry/Geometry.h>

#include "geometry/Quad.h"
#include "rendering/Framebuffer.h"
#include "rendering/Depthbuffer.h"
#include "rendering/Shader.h"
#include "rendering/Rasterizer.h"
#include "rendering/Viewport.h"
#include "common/Camera.h"
#include "GlutDemoApp.h"

// OpenGL texture Id
unsigned int texture;
unsigned int width = 640, height = 480;

class Demo04 : public GlutDemoApp
{
public:
    Demo04() : GlutDemoApp("Demo 04 - Transparency", 640, 480), sortByDepth(false) {}

protected:
    void init() override
    {
        shaders.vertexShader = std::make_shared<render::DefaultVertexTransform>();
        shaders.fragmentShader = std::make_shared<render::InputColorShader>();

        rasterizer->setRenderOutput(renderTarget);
        rasterizer->setShaders(shaders);

        // Create 3 slightly offset quads. The render order becomes fairly important as the center quad (z=0) is added last.
        auto q = std::make_unique<geometry::Quad>(glm::vec4(0.8f, 0.2f, 0.f, 0.4f));
        q->transform = glm::translate(glm::vec3(-2, 1, 2));
        quads.emplace_back(std::move(q));

        q = std::make_unique<geometry::Quad>(glm::vec4(0.1f, 0.1f, 0.8f, 0.4f));
        q->transform = glm::translate(glm::vec3(2, 1, -2));
        quads.emplace_back(std::move(q));

        q = std::make_unique<geometry::Quad > (glm::vec4(0.1f, 0.9f, 0.f, 0.4f));
        q->transform = glm::translate(glm::vec3(0, 1, 0));
        quads.emplace_back(std::move(q));
    }

    void updateFrame(float dt) override
    {
        if (sortByDepth) {
            const glm::mat4 &viewMatrix = camera->getViewMatrix();
            std::sort(quads.begin(), quads.end(),
                      [&viewMatrix](const std::unique_ptr<geometry::Quad> &a, const std::unique_ptr<geometry::Quad> &b) {
                          // Calculates the quad's position in eye coordinates.
                          const glm::vec4 posA = viewMatrix * a->transform * glm::vec4(0, 0, 0, 1);
                          const glm::vec4 posB = viewMatrix * b->transform * glm::vec4(0, 0, 0, 1);

                          // Compares the distance from the eye.
                          return posA.z < posB.z;
                      });
        }
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
            for (auto quad = quads.begin(); quad != quads.end(); ++quad) {
                dvt->modelMatrix = quad->get()->transform;
                rasterizer->drawTriangles(quad->get()->getVertices(), quad->get()->getIndices());
            }
        }
        catch (const char *txt) {
            std::cerr << "Render error :\"" << txt << "\"\n";
        }
    }

    void handleKeyboard(unsigned char key, int x, int y) override
    {
        if (key == 'd') {
            sortByDepth = !sortByDepth;
            std::cout << (sortByDepth ? "S" : "Not s") << "orting by depth." << std::endl;
        }

        if (key == 'b') {
            rasterizer->toggleBoundingBoxes();
        }
    }

protected:

private:
    bool sortByDepth;
    std::vector<std::unique_ptr<geometry::Quad> > quads;
};


int main(int argc, char **argv)
{
    Demo04 demo;
    demo.run(argc, argv);

    return 0;
}
