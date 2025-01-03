#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>

#include "DemoApp.h"
#include "geometry/Geometry.h"
#include "base/Pipeline.h"
#include "rendering/Shader.h"
#include "rendering/Texture.h"
#include "util/Camera.h"
#include "util/Frustum.h"

using namespace gfx1993;
using namespace render;
using namespace glm;

class PointField : public geometry::Geometry {
public:
  PointField(int width, int depth) {
    heightmap = HeightMap::perlinNoise(width, depth);

    glm::vec4 offset(-width/2, 0, -depth/2, 0);

    for (int x = 0; x < width; x++) {
      for (int z = 0; z < depth; z++) {
        render::Vertex v;
        v.color = glm::vec4(1,0,1,1);
        v.normal = glm::vec3(0);
        v.texcoord = glm::vec2(static_cast<float>(x)/width,
                               static_cast<float>(z)/depth);

        float y = heightmap->getTexel(glm::ivec2(x, z));
        v.position = glm::vec4(x, y, z, 1) + offset;

        vertices.push_back(v);
        indices.push_back(vertices.size()-1);
      }
    }

    std::cout << "Created a [" << width+1 << "x" << depth+1 << "] point field.\n";
    assert(indices.size() == vertices.size());
  }

  // Assigns each point a color whether it's inside or outside the frustum.
  void updatePoints(const util::Frustum& f) {
    for (render::Vertex& v : vertices) {
      vec4 worldSpacePosition = transform * v.position;
      if (f.isInside(glm::vec3(worldSpacePosition))) {
        v.color = glm::vec4(0,1,0,1);
      } else {
        v.color = glm::vec4(1,0,0,1);
      }
    }
  }
private:
  std::unique_ptr<render::HeightMap> heightmap;
};

class Demo07 : public DemoApp {
public:
  Demo07() : DemoApp("Demo 07 - Frustum"),
    camera0(glm::perspective(30.f, static_cast<float>(width) / height, 1.f, 100.f), vec3(0, 0, 10), 10),
    camera1(glm::perspective(30.f, static_cast<float>(width) / height, 1.f, 30.f), vec3(10, 2, 0)),
    frustum1(camera1.getProjectionMatrix(), camera1.getViewMatrix()),
    points(100, 100) {}

protected:
  void init() override {
    colorShader = std::make_shared<InputColorShader>();
    renderConfig.vertexShader =
        std::make_shared<render::DefaultVertexTransform>();
    renderConfig.fragmentShader = colorShader;

    logFrameTime = false;

    updateFrame(0);
  }

  void updateFrame(float dt) override {
    camera0.update(dt);
    camera1.update(dt);

    frustum1.update(camera1.getViewMatrix());

    points.updatePoints(frustum1);
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

    // Draw some points.
    renderConfig.fragmentShader = colorShader;
    renderConfig.pointSize = 9;
    dvt->modelMatrix = points.transform;
    rasterizer->drawPoints(renderConfig, points.getVertices(), points.getIndices());

    // Draw the frustum.
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
  util::OrbitCamera camera0;
  util::FreeCamera camera1;

  // This follows the free camera.
  util::Frustum frustum1;

  std::shared_ptr<render::InputColorShader> colorShader;
  PointField points;

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
