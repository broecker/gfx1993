#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "DemoApp.h"
#include "geometry/CubeGeometry.h"
#include "geometry/GridGeometry.h"
#include "geometry/PlyGeometry.h"
#include "geometry/Teapot.h"
#include "base/Pipeline.h"
#include "rendering/Shader.h"

using namespace gfx1993;

static float randf() {
  return static_cast<float>(rand()) / RAND_MAX;
}

static glm::vec3 randVec(const glm::vec3& min, const glm::vec3& max) {
  return glm::vec3(glm::mix(min.x, max.x, randf()),
                   glm::mix(min.y, max.y, randf()),
                   glm::mix(min.z, max.z, randf()));
}

static glm::mat4 makeRandomTransform() {
  const glm::vec3 minPos(-15);
  const glm::vec3 maxPos(15);

  // Random rotation and translation.
  glm::vec3 rotationAxis = glm::normalize(randVec(glm::vec3(-1), glm::vec3(1)));
  glm::mat4 rotate = glm::rotate(randf() * 360.f, rotationAxis);

  return rotate * glm::translate(randVec(minPos, maxPos));
}

static glm::vec4 makeRandomColor() {
  // Random color.
  float r = randf();
  float g = randf();
  float b = 1.f - r - g;
  return glm::vec4(r,g,b,1);
}

class Demo02 : public DemoApp {
public:
  Demo02() : DemoApp("Demo 02 - Hello Geometry") {}

protected:
  void init() override {
    renderConfig.vertexShader =
        std::make_shared<render::DefaultVertexTransform>();

    normalColorShader = std::make_shared<render::NormalColorShader>();
    inputColorShader = std::make_shared<render::InputColorShader>();

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

    // Clear the buffers
    renderConfig.clearBuffers(glm::vec4(0.7f, 0.7f, 0.9f, 1));

    // Draw the floor grid.
    renderConfig.fragmentShader = inputColorShader;
    rasterizer->drawLines(renderConfig, grid->getVertices(),
                          grid->getIndices());

    // Draw all the bunnies.
    renderConfig.fragmentShader = normalColorShader;
    for (const auto& bunny : bunnyList) {
      dvt->modelMatrix = bunny->transform;
      rasterizer->drawTriangles(renderConfig, bunny->getVertices(),
                                bunny->getIndices());
    }

    // Draw all the cubes.
    renderConfig.fragmentShader = inputColorShader;
    for (const auto& cube : cubes) {
      dvt->modelMatrix = cube->transform;
      rasterizer->drawTriangles(renderConfig, cube->getVertices(), cube->getIndices());
    }

  }

  void handleKeyboard(unsigned char key, const glm::ivec2& mousePosition) override {
    if (key == 'b') {
      renderConfig.drawTriangleBounds = !renderConfig.drawTriangleBounds;
      std::cout << "Drawing tri raster bounds: " << renderConfig.drawTriangleBounds << std::endl;
    }

    if (key == 'c') {
      const glm::vec3 minSize(2);
      const glm::vec3 maxSize(20);

      auto cube = std::make_unique<geometry::CubeGeometry>(glm::mix(minSize, maxSize, randf()));

      cube->transform = makeRandomTransform();
      glm::vec4 color = makeRandomColor();
      for (render::Vertex& v : cube->getMutableVertexList()) {
        v.color = color;
      }

      cubes.emplace_back(std::move(cube));
    }

    if (key == 'd') {
      cubes.clear();
    }

    if (key == 'f') {
      renderConfig.cullBackFaces = !renderConfig.cullBackFaces;
      std::cout << "Culling backfaces: " << renderConfig.cullBackFaces << std::endl;
    }

    if (key == 'g') {
      std::unique_ptr<geometry::PlyGeometry> bunny =
          std::make_unique<geometry::PlyGeometry>();

      bunny->loadPly("../models/bunny/reconstruction/bun_zipper_res3.ply");

      float randomAngle = (float)rand() / RAND_MAX;
      glm::vec3 randomAxis = glm::sphericalRand(1);

      const glm::vec4 minBounds(-15, -4, -15, 1);
      const glm::vec4 maxBounds(15, 15, 15, 1);
      const glm::vec3 minScale(35, 35, 35);
      const glm::vec3 maxScale(75, 75, 75);

      bunny->transform = glm::rotate(randomAngle, randomAxis);
      bunny->transform[3] = glm::linearRand(minBounds, maxBounds);
      bunny->transform *= glm::scale(glm::linearRand(minScale, maxScale));

      bunnyList.emplace_back(std::move(bunny));
    }

    if (key == 'G') {
      bunnyList.clear();
    }

    if (key == 't') {
      auto teapot = std::make_unique<geometry::Teapot>();
      glm::vec4 color = makeRandomColor();
      for (render::Vertex& v : teapot->getMutableVertexList()) {
        v.color = color;
      }
      teapot->transform = makeRandomTransform();
      cubes.emplace_back(std::move(teapot));
    }
  }

private:
  std::unique_ptr<geometry::GridGeometry> grid;
  std::vector<std::unique_ptr<geometry::PlyGeometry>> bunnyList;
  std::vector<std::unique_ptr<geometry::Geometry>> cubes;

  std::shared_ptr<render::FragmentShader> normalColorShader;
  std::shared_ptr<render::FragmentShader> inputColorShader;

};

int main(int argc, char **argv) {
  Demo02 demo;
  demo.run(argc, argv);

  return 0;
}
