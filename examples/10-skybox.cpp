#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "DemoApp.h"
#include "geometry/GridGeometry.h"
#include "geometry/Quad.h"
#include "geometry/Teapot.h"
#include "base/Pipeline.h"
#include "rendering/Shader.h"
#include "util/Camera.h"

using namespace gfx1993;
using namespace render;
using namespace glm;

class SkyboxVertexShader : public render::VertexShader {
public:
  VertexOut transformSingle(const Vertex &in) override {
    VertexOut out;
    out.clipPosition = vec4(in.position.x, in.position.y, 1.0, 1.0);
    out.texcoord = in.texcoord;
    out.color = normalize(in.position);

    vec4 farPlanePos = inverseViewProjection * vec4(in.position.x, in.position.y, 1.f, 1.f);
    farPlanePos /= farPlanePos.w;

    vec4 cameraPos = inverseViewProjection * vec4(0, 0, 0, 1);
	  cameraPos /= cameraPos.w;

    vec3 viewDir = normalize(vec3(farPlanePos) - vec3(cameraPos));
    out.varying[0] = vec4(viewDir, 0);
    out.color = vec4(viewDir, 0);
    return out;
  }

  mat4 inverseViewProjection;
};

class SkyboxFragmentShader : public FragmentShader {
public:
  SkyboxFragmentShader(const vec3& sky, const vec3& horizon, const vec3& ground) : 
    sky(sky), horizon(horizon), ground(ground) {}

  render::Fragment shadeSingle(const render::ShadingGeometry& in) override {
    render::Fragment out;

    vec3 viewDir = in.varying[0];
    vec3 V = normalize(viewDir);

    vec3 color = V;
    if (V.y > 0.0) {
      color = mix(horizon, sky, V.y);
    }
    else {
      color = mix(horizon, ground, -V.y);
    }

    out.color = vec4(color, 1.0);
    out.discard = false;

    return out;
  }

private:
  vec3 sky, horizon, ground;
};

class Demo10 : public DemoApp {
public:
  Demo10() : DemoApp("Demo 10 - Skybox"), skybox(vec4(1.f)) {}

protected:
  void init() override {
    fixedFunctionTransform = std::make_shared<render::DefaultVertexTransform>();
    gridShader = std::make_shared<InputColorShader>();

    skyboxVertShader = std::make_shared<SkyboxVertexShader>();
    skyboxFragShader = std::make_shared<SkyboxFragmentShader>(
      vec3(0.7f, 0.7f, 1.f), 
      vec3(0.8f, 0.8f, 0.9f), 
      vec3(0.3f, 0.3f, 0.4f));

    grid = std::make_unique<geometry::GridGeometry>();

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
  std::unique_ptr<geometry::GridGeometry> grid;

  std::shared_ptr<render::DefaultVertexTransform> fixedFunctionTransform;
  std::shared_ptr<render::FragmentShader> gridShader;
  std::shared_ptr<render::SingleColorShader> colorShader;

  std::shared_ptr<SkyboxVertexShader> skyboxVertShader;
  std::shared_ptr<SkyboxFragmentShader> skyboxFragShader;
  geometry::Quad skybox;

  geometry::Teapot teapot;
};

int main(int argc, char **argv) {
  Demo10 demo;
  demo.run(argc, argv);

  return 0;
}
