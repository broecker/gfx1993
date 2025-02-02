#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "DemoApp.h"
#include "Pipeline.h"
#include "Shader.h"

using namespace gfx1993;
using glm::vec4;

class Demo00 : public DemoApp {
public:
  Demo00() : DemoApp("Demo 00 - Hello Triangle"), rotationAngle(0) {}

protected:
  void init() override {
    // Keep a separate reference to the vertex shader so we can change the
    // transform easily.
    vertexShader = std::make_shared<DefaultVertexTransform>();
    renderConfig.vertexShader = vertexShader;
    renderConfig.fragmentShader = std::make_shared<InputColorShader>();

    // Create the triangle geometry
    vertices.push_back(Vertex(vec4( 0,  1, 0, 1), vec4(0, 0, 1, 1)));
    vertices.push_back(Vertex(vec4(-1, -1, 0, 1), vec4(1, 0, 0, 1)));
    vertices.push_back(Vertex(vec4( 1, -1, 0, 1), vec4(0, 1, 0, 1)));

    indices.push_back(0);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(1);
  }

  void updateFrame(float dt) override {
    DemoApp::updateFrame(dt);
    rotationAngle += dt * 3.f;
  }

  void renderFrame() override {
    // clear the buffers
    renderConfig.clearBuffers(vec4(0, 0, 0.2f, 1));

    // reset the render matrices
    vertexShader->modelMatrix = glm::rotate(rotationAngle, glm::vec3(0, 1, 0));
    vertexShader->viewMatrix = glm::lookAt(
        glm::vec3(0, 1, -10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    vertexShader->projectionMatrix = glm::perspective(
        glm::radians(60.f), (float)width / height, 0.2f, 100.f);

    try {
      rasterizer->drawTriangles(renderConfig, vertices, indices);
    } catch (const char *txt) {
      std::cerr << "gfx1993 error :\"" << txt << "\"\n";
    }
  }

private:
  float rotationAngle;

  VertexList vertices;
  IndexList indices;

  std::shared_ptr<DefaultVertexTransform> vertexShader;
};

int main(int argc, char **argv) {
  Demo00 demo;
  demo.run(argc, argv);

  return 0;
}
