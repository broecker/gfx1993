#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>

#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform2.hpp>

#include "rendering/Depthbuffer.h"
#include "rendering/Framebuffer.h"
#include "rendering/Pipeline.h"
#include "rendering/Rasterizer.h"
#include "rendering/Shader.h"
#include "rendering/Viewport.h"

unsigned int width = 640, height = 480;
std::string binaryName;

render::VertexList vertices;
render::IndexList indices;

std::unique_ptr<render::Rasterizer> rasterizer;
render::RenderConfig renderConfig;
std::shared_ptr<render::DefaultVertexTransform> vertexShader;

using render::Vertex;

const static int MAX_FRAMES = 36;

// A single frame -- rotates and draws the triangle.
static void frame(int counter) {
  // Clear the buffers
  renderConfig.clearBuffers(glm::vec4(0, 0, 0.2f, 1));

  // reset the render matrices
  float rotation = (float)counter / MAX_FRAMES * glm::pi<float>() * 2;

  vertexShader->modelMatrix = glm::rotate(rotation, glm::vec3(0, 1, 0));
  vertexShader->viewMatrix =
      glm::lookAt(glm::vec3(0, 1, -10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
  vertexShader->projectionMatrix =
      glm::perspective(glm::radians(60.f), (float)width / height, 0.2f, 100.f);

  try {
    rasterizer->drawTriangles(renderConfig, vertices, indices);
  } catch (const char *txt) {
    std::cerr << "Render error :\"" << txt << "\"\n";
  }
}

// Writes out a ppm image of the framebuffer.
void writeImage(int counter) {
  char filename[128];
  sprintf(filename, "%s-%02d.ppm", binaryName.c_str(), counter);

  std::ofstream file(filename);
  file << "P3" << std::endl;
  ;
  file << width << " " << height << std::endl;
  file << 255 << std::endl;

  for (unsigned int y = 0; y < renderConfig.framebuffer->getHeight(); ++y) {
    for (unsigned int x = 0; x < renderConfig.framebuffer->getWidth(); ++x) {
      const glm::vec4 &c =
          renderConfig.framebuffer
              ->getPixels()[x + y * renderConfig.framebuffer->getWidth()];
      file << (int)(c.r * 255) << " " << (int)(c.g * 255) << " "
           << (int)(c.b * 255) << " ";
    }
    file << std::endl;
  }

  std::clog << "Wrote " << filename << std::endl;
  file.close();
}

int main(int argc, char **argv) {
  binaryName = argv[0];

  srand(time(0));

  // Setup the rasterizer and the shaders.
  rasterizer = std::make_unique<render::Rasterizer>();
  renderConfig.viewport =
      std::make_shared<render::Viewport>(0, 0, width, height);
  renderConfig.framebuffer =
      std::make_shared<render::Framebuffer>(width, height);
  renderConfig.depthbuffer =
      std::make_shared<render::Depthbuffer>(width, height);

  // Keep vertex shader separate so we can easily modify the transformation.
  vertexShader = std::make_shared<render::DefaultVertexTransform>();
  renderConfig.vertexShader = vertexShader;
  renderConfig.fragmentShader = std::make_shared<render::InputColorShader>();

  // Create the triangle geometry
  vertices.push_back(Vertex(glm::vec4(-5, 0, 0, 1), glm::vec3(1, 0, 0),
                            glm::vec4(0, 0, 1, 1), glm::vec2(0, 0)));
  vertices.push_back(Vertex(glm::vec4(0, 5, 0, 1), glm::vec3(1, 0, 0),
                            glm::vec4(1, 0, 0, 1), glm::vec2(0, 0)));
  vertices.push_back(Vertex(glm::vec4(5, 0, 0, 1), glm::vec3(1, 0, 0),
                            glm::vec4(0, 1, 0, 1), glm::vec2(0, 0)));

  indices.push_back(0);
  indices.push_back(2);
  indices.push_back(1);
  indices.push_back(2);
  indices.push_back(0);
  indices.push_back(1);

  // Draw our frames.
  for (int i = 0; i < MAX_FRAMES; ++i) {
    frame(i);
    writeImage(i);
  }

  return 0;
}
