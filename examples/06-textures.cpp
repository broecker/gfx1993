#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "DemoApp.h"
#include "geometry/Quad.h"
#include "base/Pipeline.h"
#include "rendering/Shader.h"
#include "rendering/Texture.h"

using namespace gfx1993;

class TextureShader : public render::FragmentShader {
public:
  TextureShader() : texture(nullptr) {}

  render::Fragment shadeSingle(const render::ShadingGeometry &in) override {
    render::Fragment fragment;
    fragment.color = glm::vec4(1,0,1,1);

    if (texture) {
      fragment.color = texture->getTexel(in.texcoord,  mode);
    }
    return fragment;
  }

  inline void setTexture(std::shared_ptr<render::Texture> texture) {
    std::cout << "[TexShader] Updating texture to " << texture->getId() << " \n";
    this->texture = texture;
  }

  void switchLookup() {
    if (mode == render::Texture::CLAMP) {
      mode = render::Texture::REPEAT;
    } else {
      mode = render::Texture::CLAMP;
    }

    std::cout << "[TexShader] Switched lookup to " << mode << std::endl;
  }

private:
  std::shared_ptr<render::Texture> texture;

  render::Texture::LookupMode mode = render::Texture::REPEAT;
};

class TexCoordShader : public render::FragmentShader {
public:
  render::Fragment shadeSingle(const render::ShadingGeometry &in) override {
    render::Fragment fragment;
    fragment.color = glm::vec4(in.texcoord, 0.f, 1.f);
    return fragment;
  }
};

class Demo06 : public DemoApp {
public:
  Demo06() : DemoApp("Demo 06 - Textures") {}

protected:
  void init() override {
    textureShader = std::make_shared<TextureShader>();
    texCoordShader = std::make_shared<TexCoordShader>();
    missingTextureShader = std::make_shared<render::SingleColorShader>(glm::vec4(1.0, 0.0, 1.0, 1.0));

    renderConfig.vertexShader =
        std::make_shared<render::DefaultVertexTransform>();
    renderConfig.fragmentShader = texCoordShader;

    quad = std::make_unique<geometry::Quad>(glm::vec4(1));

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
    render::DefaultVertexTransform *dvt =
        dynamic_cast<render::DefaultVertexTransform *>(
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
      texture = render::Texture::makeCheckerboard(
          64, 64, 8, glm::vec4(1.f, 0.f, 0.f, 1.f),
          glm::vec4(1.f, 1.f, 0.f, 1.f));
      updateTexture();
    }

    if (key == 'l') {
      texture = render::Texture::loadPPM("./lenna.ppm");
      updateTexture();
    }

    // Set texture coord debugging shader.
    if (key == 't') {
      renderConfig.fragmentShader = texCoordShader;
      texture = nullptr;
    }

    if (key == 'm') {
      textureShader->switchLookup();
    }

    if (key == 'n') {
      texture = render::Texture::perlinNoise(128, 128, glm::vec2(10));
      updateTexture();
    }
  }

private:
  std::unique_ptr<geometry::Quad> quad;
  std::shared_ptr<TextureShader> textureShader;
  std::shared_ptr<TexCoordShader> texCoordShader;
  std::shared_ptr<render::SingleColorShader> missingTextureShader;

  std::shared_ptr<render::Texture> texture = nullptr;


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
