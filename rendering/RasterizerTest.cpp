#include <GUnit.h>

#include <limits>

#include <iostream>
#include <glm/ext.hpp>
#include <glm/gtc/epsilon.hpp>

#include "Rasterizer.h"
#include "Shader.h"
#include "Viewport.h"
#include "../base/Pipeline.h"

namespace gfx1993 {
namespace render {

using glm::ivec2;
using glm::vec4;

// Simple pass-through shader.
class TestVertShader : public VertexShader {
public:
  VertexOut transformSingle(const Vertex &in) override {
    return VertexOut{.clipPosition = in.position, .color = in.color };
  }
};

class TestFragShader : public FragmentShader {
public:
  Fragment shadeSingle(const ShadingGeometry& in) override {
    return Fragment {.color = in.color, .discard=false };
  }
};

class DiscardFragShader : public FragmentShader {
public:
  Fragment shadeSingle(const ShadingGeometry &in) {
    Fragment out;
    out.color = vec4(1,0,1,1);
    out.discard = true;
    return out;
  }
};

constexpr int TEST_W=4;
constexpr int TEST_H=4;
constexpr int TEST_PIXELS=TEST_W*TEST_H;

namespace {
void printRenderedImage(std::shared_ptr<Framebuffer> frameBuffer,
                        std::shared_ptr<Depthbuffer> depthBuffer) {
  std::cout << "Rendered image:\n";
  for (unsigned int x = 0; x < TEST_W; ++x) {
    for (unsigned int y = 0; y < TEST_H; ++y) {
      const glm::vec4& px = frameBuffer->getPixel(x,y);
      std::cout << px.x << "," << px.y << "," << px.z << " ";
    }
    std::cout << std::endl;
  }

  std::cout << "Rendered depth:\n";
  for ( int x = 0; x < TEST_W; ++x) {
    for (unsigned int y = 0; y < TEST_H; ++y) {
      std::cout << depthBuffer->getDepth(x,y) << " ";
    }
    std::cout << std::endl;
  }
}
}  // namespace

GTEST("Rasterizer Test") {
  Rasterizer rasterizer;

  auto frameBuffer = std::make_shared<Framebuffer>(TEST_W, TEST_H);
  auto depthBuffer =
  std::make_shared<Depthbuffer>(TEST_W, TEST_H);
  auto viewport = std::make_shared<Viewport>(0,0,TEST_W, TEST_H);

  SHOULD("Draw screen filling quad") {
    RenderConfig config;
    config.framebuffer = frameBuffer;
    config.depthbuffer = nullptr;
    config.viewport = viewport;
    config.depthWrite = false;
     
    config.vertexShader = std::make_shared<DefaultVertexTransform>();
    config.fragmentShader = std::make_shared<SingleColorShader>(vec4(1,0,0,1));

    rasterizer.drawScreenFillingQuad(config);

    EXPECT(rasterizer.getDebugInfo().screenFillingQuad.processed == 1);
    EXPECT(rasterizer.getDebugInfo().screenFillingQuad.drawn == 1);
    EXPECT(rasterizer.getDebugInfo().screenFillingQuad.fragmentsDrawn == TEST_PIXELS);
    EXPECT(frameBuffer->getPixel(ivec2(0,0)) == vec4(1,0,0,1));
  }

  SHOULD("Not draw to depth buffer when depth write is disabled") {
    RenderConfig config;
    config.framebuffer = frameBuffer;
    config.depthbuffer = depthBuffer;
    config.viewport = viewport;
    config.depthWrite = false;
     
    depthBuffer->clear(2.0f);

    config.vertexShader = std::make_shared<DefaultVertexTransform>();
    config.fragmentShader = std::make_shared<SingleColorShader>(vec4(1,0,0,1));

    rasterizer.drawScreenFillingQuad(config);

    EXPECT(rasterizer.getDebugInfo().screenFillingQuad.processed == 1);
    EXPECT(rasterizer.getDebugInfo().screenFillingQuad.drawn == 1);
    EXPECT(rasterizer.getDebugInfo().screenFillingQuad.fragmentsDrawn == TEST_PIXELS);

    for (int x = 0; x < TEST_W; ++x) {
      for (int y = 0; y < TEST_H; ++y) {
        const ivec2 px(x, y);
        EXPECT(frameBuffer->getPixel(px) == vec4(1,0,0,1));
        EXPECT(depthBuffer->getDepth(px) == 2.0f);
      }
    }
  }

  SHOULD("Draw to buffers when depth test is disabled") {
    RenderConfig config;
    config.framebuffer = frameBuffer;
    config.depthbuffer = depthBuffer;
    config.viewport = viewport;
    // That's an odd combination though.
    config.depthWrite = true;
    config.depthTest = false;
     
    depthBuffer->clear(-1.0f);

    config.vertexShader = std::make_shared<DefaultVertexTransform>();
    config.fragmentShader = std::make_shared<SingleColorShader>(vec4(1,0,0,1));

    rasterizer.drawScreenFillingQuad(config);

    EXPECT(rasterizer.getDebugInfo().screenFillingQuad.processed == 1);
    EXPECT(rasterizer.getDebugInfo().screenFillingQuad.drawn == 1);
    EXPECT(rasterizer.getDebugInfo().screenFillingQuad.fragmentsDrawn == TEST_PIXELS);

    for (int x = 0; x < TEST_W; ++x) {
      for (int y = 0; y < TEST_H; ++y) {
        const ivec2 px(x, y);

        EXPECT(frameBuffer->getPixel(px) == vec4(1,0,0,1));
        // Not sure what the actual value is.
        EXPECT(depthBuffer->getDepth(px) != -1);
      }
    }
  }

  SHOULD("Not rasterize discarded fragments.") {
    RenderConfig config;
    config.framebuffer = frameBuffer;
    config.depthbuffer = depthBuffer;
    config.viewport = viewport;
     
    frameBuffer->clear(vec4(1,0,0,1));
    depthBuffer->clear();

    config.vertexShader = std::make_shared<DefaultVertexTransform>();
    config.fragmentShader = std::make_shared<DiscardFragShader>();

    rasterizer.drawScreenFillingQuad(config);

    EXPECT(rasterizer.getDebugInfo().screenFillingQuad.processed == 1);
    EXPECT(rasterizer.getDebugInfo().screenFillingQuad.drawn == 1);
    EXPECT(rasterizer.getDebugInfo().screenFillingQuad.fragmentsDrawn == 0);

    for (int x = 0; x < TEST_W; ++x) {
      for (int y = 0; y < TEST_H; ++y) {
        const ivec2 px(x, y);

        EXPECT(frameBuffer->getPixel(px) == vec4(1,0,0,1));
        EXPECT(depthBuffer->getDepth(px) == std::numeric_limits<float>::max());
      }
    }
  }

  SHOULD("Rasterize only to viewport area") {
    RenderConfig config;
    config.framebuffer = frameBuffer;
    config.depthbuffer = depthBuffer;
    config.viewport = std::make_shared<Viewport>(1, 1, TEST_W-2, TEST_H-2);

    const vec4 red = vec4(1,0,0,1);
    const vec4 blue = vec4(0,1,0,1);

    frameBuffer->clear(red);
    depthBuffer->clear(100.f);


    config.vertexShader = std::make_shared<DefaultVertexTransform>();
    config.fragmentShader = std::make_shared<SingleColorShader>(blue);

    rasterizer.drawScreenFillingQuad(config);

    EXPECT(rasterizer.getDebugInfo().screenFillingQuad.processed == 1);
    EXPECT(rasterizer.getDebugInfo().screenFillingQuad.drawn == 1);
    EXPECT(rasterizer.getDebugInfo().screenFillingQuad.fragmentsDrawn == (TEST_W-2)*(TEST_H-2));

    // EXPECT(frameBuffer->getPixel(0,0) == red);
    // EXPECT(frameBuffer->getPixel(1,0) == blue);

    // First and last row are outside the viewport and should be all red.
    for (int x = 0; x < TEST_W; ++x) {
      EXPECT(frameBuffer->getPixel(x, 0) == red);
      EXPECT(frameBuffer->getPixel(x, TEST_H-1) == red);
      EXPECT(depthBuffer->getDepth(x, 0) == 100.f);
      EXPECT(depthBuffer->getDepth(x, TEST_H-1) == 100.f);
    }

    // Other rows should have a 1px border.
    for (int y = 1; y < TEST_H-2; ++y) {
      EXPECT(frameBuffer->getPixel(0, y) == red);
      EXPECT(frameBuffer->getPixel(TEST_W-1, y) == red);
      EXPECT(depthBuffer->getDepth(0, y) == 100.f);
      EXPECT(depthBuffer->getDepth(TEST_W-1, y) == 100.f);

      for (int x = 1; x < TEST_W-2; ++x) {
        EXPECT(frameBuffer->getPixel(x, y) == blue);
        EXPECT(depthBuffer->getDepth(x, y) < 1.f);
      }
    }
  }

  SHOULD("Draw fat points") {
    RenderConfig config;
    config.framebuffer = frameBuffer;
    config.depthbuffer = depthBuffer;
    config.viewport = viewport;

    const vec4 red = vec4(1,0,0,1);

    frameBuffer->clear(red);
    depthBuffer->clear(10.f);

    config.vertexShader = std::make_shared<TestVertShader>();
    config.fragmentShader = std::make_shared<TestFragShader>();

    config.pointSize = 3;

    // Draw a single black point at the center of the screen.
    VertexList points{ Vertex(vec4(0,0,-1,1)) };
    IndexList indices{ 0 };  

    ASSERT(config.isValid());

    rasterizer.drawPoints(config, points, indices);

    printRenderedImage(frameBuffer, depthBuffer);

    // Top row and left column are red (clear color) and depth is is not set.
    for (int x = 0; x < TEST_W; ++x) {
      EXPECT(frameBuffer->getPixel(x, 0) == red);
      EXPECT(depthBuffer->getDepth(x, 0) == 10);
    }
    for (int y = 0; y < TEST_H; ++y) {
      EXPECT(frameBuffer->getPixel(0, y) == red);
      EXPECT(depthBuffer->getDepth(0, y) == 10);
    }

    EXPECT(rasterizer.getDebugInfo().points.drawn == 1);
    EXPECT(rasterizer.getDebugInfo().points.fragmentsDrawn == 9);
    

    // The 3x3 pixel area (i.e. the point) is filled with black and depth is
    // set.
    const vec4 black(0,0,0,1);
    for (int x = 0; x < 3; ++x) {
      for (int y = 0; y < 3; ++y) {
        EXPECT(frameBuffer->getPixel(1+x, 1+y) == black);
        EXPECT(depthBuffer->getDepth(1+x, 1+y) == 0);
      }
    }
  }
}

}  // namespace render
}  // namespace gfx1993
