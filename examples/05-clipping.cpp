#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>

#include <geometry/Geometry.h>
#include <geometry/GridGeometry.h>
#include <rendering/Pipeline.h>

#include "GlutDemoApp.h"
#include "common/Camera.h"
#include "rendering/Shader.h"

// Creates random geometry within a bounding volume.
class BoundedGeometry : public geometry::Geometry {
public:
  BoundedGeometry(const glm::vec3 &min, const glm::vec3 &max)
      : maxBounds(max), minBounds(min) {}

  virtual ~BoundedGeometry() = default;

protected:
  glm::vec3 maxBounds, minBounds;

  static glm::vec4 getRandomColor() {
    float r = glm::linearRand(0.f, 1.f);
    float g = glm::linearRand(0.f, 1.f);
    float b = 1.f - r - g;
    float a = 1.f;
    return glm::vec4(r, g, b, a);
  }
};

class RandomLinesGeometry : public BoundedGeometry {
public:
  RandomLinesGeometry(const glm::vec3 &min, const glm::vec3 &max)
      : BoundedGeometry(min, max) {}

  void add() {
    const glm::vec4 a(glm::linearRand(minBounds.x, maxBounds.x),
                      glm::linearRand(minBounds.y, maxBounds.y),
                      glm::linearRand(minBounds.z, maxBounds.z), 1.f);
    const glm::vec4 b(glm::linearRand(minBounds.x, maxBounds.x),
                      glm::linearRand(minBounds.y, maxBounds.y),
                      glm::linearRand(minBounds.z, maxBounds.z), 1.f);
    const glm::vec3 zero(0);

    vertices.push_back(render::Vertex(a, zero, getRandomColor(), glm::vec2(0)));
    vertices.push_back(render::Vertex(b, zero, getRandomColor(), glm::vec2(1)));
    indices.push_back(vertices.size() - 2);
    indices.push_back(vertices.size() - 1);
  }

  void clear() {
    vertices.clear();
    indices.clear();
  }
};

class RandomPointsGeometry : public BoundedGeometry {
public:
  RandomPointsGeometry(const glm::vec3 &min, const glm::vec3 &max)
      : BoundedGeometry(min, max) {}

  void add() {
    const glm::vec4 p(glm::linearRand(minBounds.x, maxBounds.x),
                      glm::linearRand(minBounds.y, maxBounds.y),
                      glm::linearRand(minBounds.z, maxBounds.z), 1.f);
    const glm::vec3 zero(0);

    vertices.push_back(render::Vertex(p, zero, getRandomColor(), glm::vec2(0)));
    indices.push_back(vertices.size() - 1);
  }

  void clear() {
    vertices.clear();
    indices.clear();
  }
};

class RandomTriangleGeometry : public BoundedGeometry {
public:
  RandomTriangleGeometry(const glm::vec3 &min, const glm::vec3 &max)
      : BoundedGeometry(min, max) {}

  void add(const glm::vec3 &a_, const glm::vec3 &b_, const glm::vec3 &c_,
           bool doublesided = true) {
    const glm::vec4 a(a_, 1);
    const glm::vec4 b(b_, 1);
    const glm::vec4 c(c_, 1);
    const glm::vec3 zero(0);

    const glm::vec4 red(1, 0, 0, 1);
    const glm::vec4 grn(0, 1, 0, 1);
    const glm::vec4 blu(0, 0, 1, 1);

    vertices.push_back(render::Vertex(a, zero, red, glm::vec2(0)));
    vertices.push_back(render::Vertex(b, zero, grn, glm::vec2(1)));
    vertices.push_back(render::Vertex(c, zero, blu, glm::vec2(1)));

    indices.push_back(vertices.size() - 3);
    indices.push_back(vertices.size() - 2);
    indices.push_back(vertices.size() - 1);

    if (doublesided) {
      indices.push_back(vertices.size() - 3);
      indices.push_back(vertices.size() - 1);
      indices.push_back(vertices.size() - 2);
    }
  }

  void add() {
    const glm::vec3 a(glm::linearRand(minBounds.x, maxBounds.x),
                      glm::linearRand(minBounds.y, maxBounds.y),
                      glm::linearRand(minBounds.z, maxBounds.z));
    const glm::vec3 b(glm::linearRand(minBounds.x, maxBounds.x),
                      glm::linearRand(minBounds.y, maxBounds.y),
                      glm::linearRand(minBounds.z, maxBounds.z));
    const glm::vec3 c(glm::linearRand(minBounds.x, maxBounds.x),
                      glm::linearRand(minBounds.y, maxBounds.y),
                      glm::linearRand(minBounds.z, maxBounds.z));
    add(a, b, c);
  }

  void clear() {
    indices.clear();
    vertices.clear();
  }
};

class ClippingPlane {
public:
  ClippingPlane(const glm::vec3& normal, float d) : normal(normal), distance(d), enabled(false) {}


  void moveAlongNormal(float delta) {
    distance += delta;
  }


  bool enabled;

private:
  glm::vec3     normal;
  float         distance;
};


class Demo05 : public GlutDemoApp {
public:
  Demo05() : GlutDemoApp("Demo 05 - Clipping") {}

protected:
  void init() override {
    renderConfig.vertexShader =
        std::make_shared<render::DefaultVertexTransform>();
    renderConfig.fragmentShader = std::make_shared<render::InputColorShader>();

    lines =
        std::make_unique<RandomLinesGeometry>(glm::vec3(-10), glm::vec3(10));
    points =
        std::make_unique<RandomPointsGeometry>(glm::vec3(-10), glm::vec3(10));
    triangles =
        std::make_unique<RandomTriangleGeometry>(glm::vec3(-10), glm::vec3(10));
    grid = std::make_unique<geometry::GridGeometry>();
    camera = std::make_unique<OrbitCamera>(glm::vec3(0, 0, 0),
                                           glm::vec3(0, 1, 0), 15.0f);

    makeLine(glm::vec3(0.f), glm::vec3(20.f, 0, 0), glm::vec4(1, 0, 0, 1));
    makeLine(glm::vec3(0.f), glm::vec3(0, 20.f, 0), glm::vec4(0, 1, 0, 1));
    makeLine(glm::vec3(0.f), glm::vec3(0, 0, 20.f), glm::vec4(0, 0, 1, 1));

    /*
    triangles->add(glm::vec3(0, 0, 20), glm::vec3(20, 0, -20),
                   glm::vec3(-20, 0, -20), false);
    */

    clippingPlane = std::make_unique<ClippingPlane>(glm::vec3(0,0,1), 1);
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

    switch (projectionMode) {
    case PERSPECTIVE:
      dvt->projectionMatrix = camera->getProjectionMatrix();
      break;
    case ORTHO:
      dvt->projectionMatrix = glm::ortho(-12.f, 12.f, -12.f, 12.f, -12.f, 12.f);
      break;
    default:
      assert(false);
    }

    try {
      rasterizer->drawLines(renderConfig, lines->getVertices(),
                            lines->getIndices());
      rasterizer->drawPoints(renderConfig, points->getVertices(),
                             points->getIndices());
      rasterizer->drawTriangles(renderConfig, triangles->getVertices(),
                                triangles->getIndices());

      if (drawGrid) {
        rasterizer->drawLines(renderConfig, grid->getVertices(),
                              grid->getIndices());
      }
      rasterizer->drawLines(renderConfig, lineVertices, lineIndices);
    } catch (const std::string &error) {
      std::cerr << "Render error :\"" << error << "\"\n";
    }
  }

  void handleKeyboard(unsigned char key, int x, int y) override {
    if (key == 'l') {
      for (int i = 0; i < 10; ++i) {
        lines->add();
      }
    }

    if (key == 't') {
      triangles->add();
    }

    if (key == 'p') {
      for (int i = 0; i < 10000; ++i) {
        points->add();
      }
    }

    if (key == 'g') {
      drawGrid = !drawGrid;
    }

    if (key == 'c') {
      lines->clear();
      points->clear();
      triangles->clear();
    }

    if (key == 'b') {
      renderConfig.drawTriangleBounds = !renderConfig.drawTriangleBounds;
    }

    if (key == 'o') {
      if (projectionMode == PERSPECTIVE) {
        projectionMode = ORTHO;
      } else {
        projectionMode = PERSPECTIVE;
      }
    }

    if (key == 'p') {
      clippingPlane->enabled = !clippingPlane->enabled;
    }

    if (key == 'z') {
      clippingPlane->moveAlongNormal(0.1f);
    }
    if (key == 'Z') {
      clippingPlane->moveAlongNormal(-0.1f);
    }

  }

protected:
private:
  std::unique_ptr<RandomLinesGeometry> lines;
  std::unique_ptr<RandomPointsGeometry> points;
  std::unique_ptr<RandomTriangleGeometry> triangles;
  std::unique_ptr<geometry::GridGeometry> grid;

  std::unique_ptr<ClippingPlane> clippingPlane;

  render::VertexList lineVertices;
  render::IndexList lineIndices;

  enum ProjectionMode { PERSPECTIVE, ORTHO } projectionMode = PERSPECTIVE;

  bool drawGrid = true;

  void makeLine(const glm::vec3 &a, const glm::vec3 &b,
                const glm::vec4 &color) {
    render::Vertex p(glm::vec4(a, 1));
    p.color = color;
    render::Vertex q(glm::vec4(b, 1));
    q.color = color;

    lineVertices.push_back(p);
    lineVertices.push_back(q);
    lineIndices.push_back(lineVertices.size() - 2);
    lineIndices.push_back(lineVertices.size() - 1);
  }
};

int main(int argc, char **argv) {
  Demo05 demo;
  demo.run(argc, argv);

  return 0;
}
