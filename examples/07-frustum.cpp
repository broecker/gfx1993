#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "DemoApp.h"
#include "geometry/GridGeometry.h"
#include "base/Pipeline.h"
#include "rendering/Shader.h"
#include "util/Camera.h"
#include "util/Frustum.h"

using namespace gfx1993;
using namespace render;
using namespace glm;

class Demo07 : public DemoApp {
public:
  Demo07() : DemoApp("Demo 07 - Frustum"),
    camera0(glm::perspective(30.f, static_cast<float>(width) / height, 1.f, 30.f), vec3(0, 0, 10), 10),
    camera1(glm::perspective(30.f, static_cast<float>(width) / height, 1.f, 70.f), vec3(10, 2, 0)),
    frustum0(camera0.getProjectionMatrix(), glm::mat4(1)),
    frustum1(camera1.getProjectionMatrix(), camera1.getViewMatrix()) {}

protected:
  void init() override {
    gridShader = std::make_shared<InputColorShader>();
    redShader = std::make_shared<render::SingleColorShader>(vec4(1,0,0,1));
    blueShader = std::make_shared<render::SingleColorShader>(vec4(0,0,1,1));

    renderConfig.vertexShader =
        std::make_shared<render::DefaultVertexTransform>();
    renderConfig.fragmentShader = gridShader;

    grid = std::make_unique<geometry::GridGeometry>();
    logFrameTime = false;
  }

  void updateFrame(float dt) override {
    camera0.update(dt);
    camera1.update(dt);

    frustum0.update(camera0.getViewMatrix());
    frustum1.update(camera1.getViewMatrix());
  }

  void renderFrame() override {
    // Clear the buffers
    renderConfig.clearBuffers(glm::vec4(0.7f, 0.7f, 0.9f, 1));

    // reset the render matrices
    render::DefaultVertexTransform *dvt =
        dynamic_cast<render::DefaultVertexTransform *>(
            renderConfig.vertexShader.get());

    dvt->modelMatrix = glm::mat4(1.f);
    dvt->viewMatrix = getActiveCamera()->getViewMatrix();
    dvt->projectionMatrix = getActiveCamera()->getProjectionMatrix();

    // Clear the buffers
    renderConfig.clearBuffers(glm::vec4(0.7f, 0.7f, 0.9f, 1));

    // Draw the floor grid.
    renderConfig.fragmentShader = gridShader;
    rasterizer->drawLines(renderConfig, grid->getVertices(),
                          grid->getIndices());

    // Draw the frustums.
    renderConfig.fragmentShader = redShader;
    rasterizer->drawLines(renderConfig, frustum0.getVertices(), frustum0.getIndices());
    renderConfig.fragmentShader = blueShader;
    rasterizer->drawLines(renderConfig, frustum1.getVertices(), frustum1.getIndices());
  }

  void handleKeyboard(unsigned char key, const glm::ivec2& mouse) override { 
    glm::vec3 delta(0.f);
    switch (key) {
      case 'c':
        activeCamera = 1 - activeCamera;
        std::cout << "Active camera: " << activeCamera << std::endl;
        break;
      case '1':
        activeCamera = 0;
        break;
      case '2':
        activeCamera = 1;
        break;
      case 'w':
        delta.z = 1;
        break;
      case 's':
        delta.z = -1;
        break;
      case 'a':
        delta.x = -1;
        break;
      case 'd':
        delta.x = 1;
        break;
      case 'q':
        delta.y = 1;
        break;
      case 'z':
        delta.y = -1;
        break;
      default:
        break;
    };

    if (glm::dot(delta, delta) > 0) {
      getActiveCamera()->handleInputTranslate(delta);
    }
  }

  void handleMotion(const glm::ivec2& newMousePosition) override {
    const glm::ivec2 delta = newMousePosition - mousePosition;
    this->mousePosition = newMousePosition;
    getActiveCamera()->handleInputRotate(glm::vec3(delta.y, delta.x, 0));
  }

private:
  std::unique_ptr<geometry::GridGeometry> grid;
  util::OrbitCamera camera0;
  util::FreeCamera camera1;

  util::Frustum frustum0;
  util::Frustum frustum1;

  std::shared_ptr<render::InputColorShader> gridShader;
  std::shared_ptr<render::SingleColorShader> redShader, blueShader;

  int activeCamera = 0;

  util::Camera* getActiveCamera() {
    if (activeCamera == 0) {
      return &camera0;
    } else {
      return &camera1;
    }
  }
};

int main(int argc, char **argv) {
  Demo07 demo;
  demo.run(argc, argv);

  return 0;
}
