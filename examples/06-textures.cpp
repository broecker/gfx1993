#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "DemoApp.h"
#include "Quad.h"
#include "Pipeline.h"
#include "Shader.h"
#include "Texture.h"

using namespace gfx1993;

class Demo06 : public DemoApp {
public:
  Demo06() : DemoApp("Demo 06 - Textures") {}

protected:
  void init() override {
    textureShader = std::make_shared<TextureShader>(Texture::makeCheckerboard(32, 32, 4, glm::vec4(1,0,0,1), glm::vec4(1,1,0,1)));
    texCoordShader = std::make_shared<UVShader>();
    missingTextureShader = std::make_shared<SingleColorShader>(glm::vec4(1.0, 0.0, 1.0, 1.0));

    renderConfig.vertexShader =
        std::make_shared<DefaultVertexTransform>();
    renderConfig.fragmentShader = texCoordShader;

    quad = std::make_unique<Quad>(glm::vec4(1));

    // Put the quad on the floor.
    quad->transform = glm::rotate(glm::radians(90.f), glm::vec3(1.f, 0.f, 0.f));
    // And scale it up by 5
    quad->transform *= glm::scale(glm::vec3(5));

    // And change the texture coordinates from [0..1] to [-1..2]
    for (auto& v : quad->getMutableVertexList()) {
      v.texcoord = v.texcoord * glm::vec2(3) - glm::vec2(1);
    }
  }

  void renderFrame() override {
    // Clear the buffers
    renderConfig.clearBuffers(glm::vec4(0.7f, 0.7f, 0.9f, 1));

    // reset the render matrices
    DefaultVertexTransform *dvt =
        dynamic_cast<DefaultVertexTransform *>(
            renderConfig.vertexShader.get());

    dvt->modelMatrix = glm::mat4(1.f);
    dvt->viewMatrix = camera->getViewMatrix();
    dvt->projectionMatrix = camera->getProjectionMatrix();

    try {
      dvt->modelMatrix = quad->transform;
      rasterizer->drawTriangles(renderConfig, quad->getVertices(),
                                quad->getIndices());
    } catch (const char *txt) {
      std::cerr << "Render error :\"" << txt << "\"\n";
    }
  }

  void handleKeyboard(unsigned char key, const glm::ivec2& mouse) override {
    DemoApp::handleKeyboard(key, mouse);

    if (key == 'c') {
      texture = Texture::makeCheckerboard(
          64, 64, 8, glm::vec4(1.f, 0.f, 0.f, 1.f),
          glm::vec4(1.f, 1.f, 0.f, 1.f));
      updateTexture();
    }

    if (key == 'l') {
      texture = Texture::loadPPM("./lenna.ppm");
      updateTexture();
    }

    // Set texture coord debugging shader.
    if (key == 't') {
      renderConfig.fragmentShader = texCoordShader;
      texture = nullptr;
    }

    if (key == 'm') {
      if (textureShader->mode == Texture::LookupMode::CLAMP) {
        textureShader->mode = Texture::LookupMode::REPEAT;
      } else {
        textureShader->mode = Texture::LookupMode::CLAMP;
      }
    }

    if (key == 'n') {
      texture = Texture::perlinNoise(128, 128, glm::vec2(10));
      updateTexture();
    }
  }

private:
  std::unique_ptr<Quad>             quad;
  std::shared_ptr<TextureShader>      textureShader;
  std::shared_ptr<UVShader>           texCoordShader;
  std::shared_ptr<SingleColorShader>  missingTextureShader;

  std::shared_ptr<Texture> texture = nullptr;


  void updateTexture() {
    if (texture) {
        textureShader->setTexture(texture);
        renderConfig.fragmentShader = textureShader;
      } else {
        renderConfig.fragmentShader = missingTextureShader;
      }
  }
};

int main(int argc, char **argv) {
  Demo06 demo;
  demo.run(argc, argv);

  return 0;
}
