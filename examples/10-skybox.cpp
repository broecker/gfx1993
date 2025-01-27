#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "DemoApp.h"
#include "Geometry.h"
#include "Teapot.h"
#include "Pipeline.h"
#include "Shader.h"
#include "Camera.h"

using namespace gfx1993;
using namespace glm;

class Demo10 : public DemoApp {
public:
  Demo10() : DemoApp("Demo 10 - Skybox"), skybox(vec4(1.f)) {}

protected:
  void init() override {
    fixedFunctionTransform = std::make_shared<DefaultVertexTransform>();
    gridShader = std::make_shared<InputColorShader>();

    skyboxVertShader = std::make_shared<SkyboxVertexShader>();
    skyboxFragShader = std::make_shared<SkyboxFragmentShader>(
      vec3(0.7f, 0.7f, 1.f), 
      vec3(0.8f, 0.8f, 0.9f), 
      vec3(0.3f, 0.3f, 0.4f));

    grid = std::make_unique<GridGeometry>();

    teapot.makeIndicesForPointCloud();
    colorShader = std::make_unique<SingleColorShader>(vec4(1,0,1,1));
  }

  void renderFrame() override {
    // Clear the buffers.
    {
      auto clearProf = rasterizer->getProfile().startTiming("app.clearBuffers");
      renderConfig.clearBuffers(glm::vec4(1, 0, 0, 1));
    }

    skyboxVertShader->inverseViewProjection = glm::inverse(camera->getProjectionMatrix() * camera->getViewMatrix());

    renderConfig.depthbuffer = nullptr;
    renderConfig.vertexShader = skyboxVertShader;
    renderConfig.fragmentShader = skyboxFragShader;
    rasterizer->drawTriangles(renderConfig, skybox.getVertices(), skybox.getIndices());
    
    // Reset the render matrices.
    fixedFunctionTransform->modelMatrix = glm::mat4(1.f);
    fixedFunctionTransform->viewMatrix = camera->getViewMatrix();
    fixedFunctionTransform->projectionMatrix = camera->getProjectionMatrix();

    // Draw the floor grid.
    renderConfig.depthWrite = true;
    renderConfig.depthTest = true;
    renderConfig.vertexShader = fixedFunctionTransform;
    renderConfig.fragmentShader = gridShader;
    rasterizer->drawLines(renderConfig, grid->getVertices(),
                          grid->getIndices());

    renderConfig.fragmentShader = colorShader;
    rasterizer->drawPoints(renderConfig, teapot.getVertices(), teapot.getIndices());
  }

  void handleKeyboard(unsigned char key, const glm::ivec2& mouse) override { 
    if (key == '=') {
      renderConfig.pointSize += 2;
    }
    if (key == '-') {
      renderConfig.pointSize -= 2;
    }

    renderConfig.pointSize = glm::clamp(renderConfig.pointSize, 1u, 11u);
    std::cout << "Point size: " << renderConfig.pointSize << std::endl;    
  }


private:
  std::unique_ptr<GridGeometry> grid;

  std::shared_ptr<DefaultVertexTransform> fixedFunctionTransform;
  std::shared_ptr<FragmentShader> gridShader;
  std::shared_ptr<SingleColorShader> colorShader;

  std::shared_ptr<SkyboxVertexShader>   skyboxVertShader;
  std::shared_ptr<SkyboxFragmentShader> skyboxFragShader;
  Quad skybox;

  Teapot teapot;
};

int main(int argc, char **argv) {
  Demo10 demo;
  demo.run(argc, argv);

  return 0;
}
