#include <algorithm>
#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <glm/gtx/transform.hpp>

#include "geometry/Quad.h"
#include "rendering/Pipeline.h"
#include "rendering/Shader.h"
#include <rendering/Pipeline.h>

#include "GlutDemoApp.h"

class StippleShader : public render::FragmentShader {
public:
  render::Fragment &&shadeSingle(const render::ShadingGeometry &in) override {

    // Only stipple transparent fragments.
    if (in.color.a < 1 - FLT_EPSILON) {
      // Stipple effect -- creates checkerboard pattern.
      if (in.windowCoord.y % 2 == 0) {
        if (in.windowCoord.x % 2 == 0)
          return discard();
      } else {
        if (in.windowCoord.x % 2 == 1)
          return discard();
      }

      // Reset alpha to 1.
      glm::vec4 outColor(in.color);
      outColor.a = 1.f;

      return std::move(render::Fragment{outColor});
    } else {
      return std::move(render::Fragment{in.color});
    }
  }

private:
  inline static render::Fragment &&discard() {
    return std::move(render::Fragment{glm::vec4(), true});
  }
};

class Demo04 : public GlutDemoApp {
public:
  Demo04() : GlutDemoApp("Demo 04 - Transparency"), sortByDepth(false) {}

protected:
  void init() override {
    renderConfig.vertexShader =
        std::make_shared<render::DefaultVertexTransform>();

    transparencyShader = std::make_shared<render::InputColorShader>();
    stippleShader = std::make_shared<StippleShader>();
    renderConfig.fragmentShader = transparencyShader;

    renderConfig.alphaBlending = true;

    // Create 3 slightly offset quads. The render order becomes fairly important
    // as the center quad (z=0) is added last.
    auto q = std::make_unique<geometry::Quad>(glm::vec4(0.8f, 0.2f, 0.f, 0.4f));
    q->transform = glm::translate(glm::vec3(-2, 1, 2));
    quads.emplace_back(std::move(q));

    q = std::make_unique<geometry::Quad>(glm::vec4(0.1f, 0.1f, 0.8f, 0.4f));
    q->transform = glm::translate(glm::vec3(2, 1, -2));
    quads.emplace_back(std::move(q));

    q = std::make_unique<geometry::Quad>(glm::vec4(0.1f, 0.9f, 0.f, 0.4f));
    q->transform = glm::translate(glm::vec3(0, 1, 0));
    quads.emplace_back(std::move(q));

    // And one large white quad on the floor to show through the stipple.
    q = std::make_unique<geometry::Quad>(glm::vec4(1));
    q->transform = glm::translate(glm::vec3(0, -2, 0)) *
                   glm::scale(glm::vec3(10)) *
                   glm::rotate(glm::radians(90.f), glm::vec3(1, 0, 0));
    quads.emplace_back(std::move(q));
  }

  void updateFrame(float dt) override {
    GlutDemoApp::updateFrame(dt);

    if (sortByDepth) {
      const glm::mat4 &viewMatrix = camera->getViewMatrix();
      std::sort(quads.begin(), quads.end(),
                [&viewMatrix](const std::unique_ptr<geometry::Quad> &a,
                              const std::unique_ptr<geometry::Quad> &b) {
                  // Calculates the quad's position in eye coordinates.
                  const glm::vec4 posA =
                      viewMatrix * a->transform * glm::vec4(0, 0, 0, 1);
                  const glm::vec4 posB =
                      viewMatrix * b->transform * glm::vec4(0, 0, 0, 1);

                  // Compares the distance from the eye.
                  return posA.z < posB.z;
                });
    }
  }

  void renderFrame() override {
    // Clear the buffers
    renderConfig.clearBuffers(glm::vec4(0.7f, 0.7f, 0.9f, 1));

    // reset the render matrices
    render::DefaultVertexTransform *dvt =
        dynamic_cast<render::DefaultVertexTransform *>(
            renderConfig.vertexShader.get());

    dvt->modelMatrix = glm::mat4(1.f);
    dvt->viewMatrix = camera->getViewMatrix();
    dvt->projectionMatrix = camera->getProjectionMatrix();

    try {
      for (auto quad = quads.begin(); quad != quads.end(); ++quad) {
        dvt->modelMatrix = quad->get()->transform;
        rasterizer->drawTriangles(renderConfig, quad->get()->getVertices(),
                                  quad->get()->getIndices());
      }
    } catch (const char *txt) {
      std::cerr << "Render error :\"" << txt << "\"\n";
    }
  }

  void handleKeyboard(unsigned char key, int x, int y) override {
    if (key == 'd') {
      sortByDepth = !sortByDepth;
      std::cout << (sortByDepth ? "S" : "Not s") << "orting by depth."
                << std::endl;
    }

    if (key == 'a') {
      renderConfig.alphaBlending = !renderConfig.alphaBlending;
    }

    if (key == 'b') {
      renderConfig.drawTriangleBounds = !renderConfig.drawTriangleBounds;
    }

    if (key == 's') {
      if (renderConfig.fragmentShader == transparencyShader)
        renderConfig.fragmentShader = stippleShader;
      else
        renderConfig.fragmentShader = transparencyShader;
    }
  }

protected:
private:
  bool sortByDepth;
  std::vector<std::unique_ptr<geometry::Quad>> quads;

  std::shared_ptr<render::FragmentShader> transparencyShader;
  std::shared_ptr<render::FragmentShader> stippleShader;
};

int main(int argc, char **argv) {
  Demo04 demo;
  demo.run(argc, argv);

  return 0;
}
