#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "GlutDemoApp.h"
#include "geometry/GridGeometry.h"
#include "geometry/PlyGeometry.h"
#include "rendering/Pipeline.h"
#include "rendering/Shader.h"

class Demo02 : public GlutDemoApp {
public:
  Demo02() : GlutDemoApp("Demo 02 - Hello Geometry") {}

protected:
  void init() override {
    renderConfig.vertexShader =
        std::make_shared<render::DefaultVertexTransform>();
    renderConfig.fragmentShader = std::make_shared<render::NormalColorShader>();

    grid = std::make_unique<geometry::GridGeometry>();
  }

  void renderFrame() override {
    // reset the render matrices
    render::DefaultVertexTransform *dvt =
        dynamic_cast<render::DefaultVertexTransform *>(
            renderConfig.vertexShader.get());

    dvt->modelMatrix = glm::mat4(1.f);
    dvt->viewMatrix = camera->getViewMatrix();
    dvt->projectionMatrix = camera->getProjectionMatrix();

    try {
      // Clear the buffers
      renderConfig.clearBuffers(glm::vec4(0.7f, 0.7f, 0.9f, 1));

      // Draw the floor grid.
      rasterizer->drawLines(renderConfig, grid->getVertices(),
                            grid->getIndices());

      // Draw all the bunnies.
      for (auto bunny = bunnyList.begin(); bunny != bunnyList.end(); ++bunny) {
        dvt->modelMatrix = (*bunny)->transform;
        rasterizer->drawTriangles(renderConfig, (*bunny)->getVertices(),
                                  (*bunny)->getIndices());
      }
    } catch (const char *txt) {
      std::cerr << "Render error :\"" << txt << "\"\n";
    }
  }

  void handleKeyboard(unsigned char key, int x, int y) override {
    if (key == 'b') {
      renderConfig.drawTriangleBounds = !renderConfig.drawTriangleBounds;
    }

    if (key == 'g') {
      std::unique_ptr<geometry::PlyGeometry> bunny =
          std::make_unique<geometry::PlyGeometry>();

      bunny->loadPly("../models/bunny/reconstruction/bun_zipper_res3.ply");

      float randomAngle = (float)rand() / RAND_MAX;
      glm::vec3 randomAxis = glm::sphericalRand(1);

      const glm::vec4 minBounds(-15, -4, -15, 1);
      const glm::vec4 maxBounds(15, 15, 15, 1);
      const glm::vec3 minScale(15, 15, 15);
      const glm::vec3 maxScale(30, 30, 30);

      bunny->transform = glm::rotate(randomAngle, randomAxis);
      bunny->transform[3] = glm::linearRand(minBounds, maxBounds);
      bunny->transform *= glm::scale(glm::linearRand(minScale, maxScale));

      bunnyList.emplace_back(std::move(bunny));
    }

    if (key == 'G') {
      bunnyList.clear();
    }
  }

private:
  std::unique_ptr<geometry::GridGeometry> grid;
  std::vector<std::unique_ptr<geometry::PlyGeometry>> bunnyList;
};

int main(int argc, char **argv) {
  Demo02 demo;
  demo.run(argc, argv);

  return 0;
}
