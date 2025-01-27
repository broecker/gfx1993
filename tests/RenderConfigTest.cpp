#include <GUnit.h>

#include <limits>

#include <glm/ext.hpp>
#include <glm/gtc/epsilon.hpp>

#include "Rasterizer.h"
#include "Shader.h"
#include "Viewport.h"
#include "Pipeline.h"

namespace gfx1993 {

using glm::ivec2;

GTEST("RenderConfiguration Test") {
  SHOULD("Check for valid configuration") {
    RenderConfig config;

    EXPECT(!config.isValid());
  }

  SHOULD("Expect framebuffer, viewport and shader") {
    RenderConfig config;
    config.framebuffer = std::make_shared<Framebuffer>(4,4);
    config.viewport = std::make_shared<Viewport>(ivec2(0), ivec2(4));

    config.vertexShader = std::make_shared<DefaultVertexTransform>();
    config.fragmentShader = std::make_shared<SingleColorShader>(glm::vec4(1,0,0,1));

    EXPECT(config.isValid());
  }

  SHOULD("Expect framebuffer or depthbuffer") {
    RenderConfig config;
    config.depthbuffer = std::make_shared<Depthbuffer>(4,4);
    config.viewport = std::make_shared<Viewport>(ivec2(0), ivec2(4));

    config.vertexShader = std::make_shared<DefaultVertexTransform>();
    config.fragmentShader = std::make_shared<SingleColorShader>(glm::vec4(1,0,0,1));

    EXPECT(config.isValid());
  }


  SHOULD("Have same buffer size") {
    RenderConfig config;

    config.framebuffer = std::make_shared<Framebuffer>(4,4);
    config.depthbuffer = std::make_shared<Depthbuffer>(2, 2);
    config.viewport = std::make_shared<Viewport>(ivec2(0), ivec2(4));

    config.vertexShader = std::make_shared<DefaultVertexTransform>();
    config.fragmentShader = std::make_shared<SingleColorShader>(glm::vec4(1,0,0,1));

    EXPECT(!config.isValid());
  }

  SHOULD("Have same size for buffers and viewport") {
    RenderConfig config;

    config.framebuffer = std::make_shared<Framebuffer>(4,4);
    config.depthbuffer = std::make_shared<Depthbuffer>(4, 4);
    config.viewport = std::make_shared<Viewport>(ivec2(0), ivec2(8));

    config.vertexShader = std::make_shared<DefaultVertexTransform>();
    config.fragmentShader = std::make_shared<SingleColorShader>(glm::vec4(1,0,0,1));

    EXPECT(!config.isValid());
  }

  SHOULD("Require an odd point size") {
    RenderConfig config;
    config.depthbuffer = std::make_shared<Depthbuffer>(4,4);
    config.viewport = std::make_shared<Viewport>(ivec2(0), ivec2(4));

    config.vertexShader = std::make_shared<DefaultVertexTransform>();
    config.fragmentShader = std::make_shared<SingleColorShader>(glm::vec4(1,0,0,1));

    config.pointSize = 2;

    EXPECT(!config.isValid());
  }
}

}  // namespace gfx1993
