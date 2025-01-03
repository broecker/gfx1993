#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>

#include "DemoApp.h"
#include "geometry/Geometry.h"
#include "geometry/CubeGeometry.h"
#include "base/Pipeline.h"
#include "rendering/Shader.h"
#include "rendering/Texture.h"
#include "util/BoundingVolumes.h"
#include "util/Camera.h"
#include "util/Frustum.h"

using namespace gfx1993;
using namespace render;
using namespace glm;

constexpr int TILE_SIZE = 32;
constexpr int TILE_COUNT = 3;

class PointField : public geometry::Geometry {
public:
  PointField(int width, int depth, const glm::vec3& offset) : bboxGeometry(geometry::Cube::makeLines()) {
    boundingBox.max = boundingBox.min = offset;

    // Static lighting;
    const vec3 terrainColor(0.4, 0.6, 0.3);
    const vec3 sunDirection = glm::normalize(vec3(0.3, -1, 0.4));

    vertices.resize((width)*(depth));
    // Use flat height maps and calculate perlin manually. This enables smooth
    // normals/shading between tiles.
    heightmap = render::HeightMap::makeFlat(width, depth);

    for (int x = 0; x < width; x++) {
      for (int z = 0; z < depth; z++) {

        render::Vertex v;
        v.color = glm::vec4(0.4,0.6,0.3,1);
        v.normal = glm::vec3(0);
        v.texcoord = glm::vec2((static_cast<float>(x) + offset.x)/width,
                               (static_cast<float>(z) + offset.z)/depth);

        float y = glm::perlin(v.texcoord) * 10.f;
        heightmap->setHeight(x, z, y);
        vec3 pos = glm::vec3(x, y, z) + offset;
        boundingBox.extend(pos);
        v.position = glm::vec4(pos, 1);

        // Calculate the 'previous' points in the field for dx/dz.
        vec2 t0((static_cast<float>(x-1) + offset.x) / width,
                (static_cast<float>(z-0) + offset.z) / depth);
        vec3 p0 = vec3(x-1, glm::perlin(t0) * 10.f, z) + offset;
        
        vec2 t1((static_cast<float>(x-0) + offset.x) / width,
                (static_cast<float>(z-1) + offset.z) / depth);
        vec3 p1 = vec3(x, glm::perlin(t1) * 10.f, z-1) + offset;

        v.normal = glm::normalize(glm::cross(p0 - pos, p1 - pos));

        // Static lighting.
        float L = glm::max(0.f, dot(v.normal, sunDirection));
        vec3 color = L * terrainColor;
        v.color = vec4(color, 1.0);

        size_t index = z + x*width;
        vertices[index] = v;
      }
    }

    makeIndicesForPointCloud();

    boundingBox.updateGeometry(bboxGeometry);

    std::cout << "Created a [" << width << "x" << depth << "] point field.\n";
    assert(indices.size() == vertices.size());
  }

  void setBoundingBoxColor(const vec3& color) {
    for (auto& v : bboxGeometry.getMutableVertexList()) {
      v.color = vec4(color, 1.f);
    }
  }

  const util::AABB& getBoundingBox() const { return boundingBox; }

  const geometry::Cube& getBoundingBoxGeo() const { return bboxGeometry; }

  bool            visible;

private:
  util::AABB      boundingBox;
  geometry::Cube  bboxGeometry;

  std::unique_ptr<render::HeightMap> heightmap;
};

class Demo12 : public DemoApp {
public:
  Demo12() : DemoApp("Demo 12 - Frustum / AABB Culling"), frustum(mat4(1.f), mat4(1.f)) {
      camera = std::make_unique<util::FreeCamera>(
          glm::perspective(30.f, static_cast<float>(width) / height, 1.f, 200.f), 
          vec3(0, 2, 10));

      frustum = util::Frustum(camera->getProjectionMatrix(), camera->getViewMatrix());

    }

protected:
  void init() override {
    colorShader = std::make_shared<InputColorShader>();
    renderConfig.vertexShader =
        std::make_shared<render::DefaultVertexTransform>();
    renderConfig.fragmentShader = colorShader;

    for (int x = -TILE_COUNT/2; x <= TILE_COUNT/2; ++x) {
      for (int z= -TILE_COUNT/2; z <= TILE_COUNT/2; ++z) {
        tiles.push_back(std::make_unique<PointField>(TILE_SIZE, TILE_SIZE, glm::vec3(TILE_SIZE*x,0,TILE_SIZE*z)));
      }
    }

    logFrameTime = true;
    updateFrame(0);
  }

  void updateFrame(float dt) override {
    camera->update(dt);
    frustum.update(camera->getViewMatrix());

    for (const auto& tile : tiles) {
      util::Frustum::IntersectionResult result = frustum.testIntersection(tile->getBoundingBox());

      if (result == util::Frustum::INSIDE) {
        tile->visible = true;
        tile->setBoundingBoxColor(vec3(0,1,0));
      }

      if (result == util::Frustum::INTERSECTING) {
        tile->visible = true;
        tile->setBoundingBoxColor(vec3(1,1,0));
      }

      if (result == util::Frustum::OUTSIDE) {
        tile->visible = false;
        tile->setBoundingBoxColor(vec3(1,0,0));
      }
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

    // Clear the buffers
    renderConfig.clearBuffers(glm::vec4(0.7f, 0.7f, 0.9f, 1));


    // Draw visible tiles.
    renderConfig.fragmentShader = colorShader;
    renderConfig.pointSize = 9;
    dvt->modelMatrix = glm::mat4(1.f);
    
    auto& debugInfo = rasterizer->getDebugInfo();

    for (const auto& tile : tiles) {
      debugInfo.aabbs.drawn++;
      if (!tile->visible && frustumCulling) {
        debugInfo.aabbs.backfaceCulled++;
        continue;
      }
      

      // Draw bounding box.
      renderConfig.depthWrite = false;
      renderConfig.depthTest = false;
      rasterizer->drawLines(renderConfig,
                            tile->getBoundingBoxGeo().getVertices(),
                            tile->getBoundingBoxGeo().getIndices());


      renderConfig.depthWrite = true;
      renderConfig.depthTest = true;
      rasterizer->drawPoints(renderConfig, tile->getVertices(), tile->getIndices());
    }
  }

  void handleKeyboard(unsigned char key, const glm::ivec2& mouse) override { 
    glm::vec3 delta(0.f);
    switch (key) {
      case 'c':
        frustumCulling = !frustumCulling;
        std::cout << "Cull tiles: " << frustumCulling << std::endl;
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
      camera->handleInputTranslate(delta);
    }
  }

  void handleMotion(const glm::ivec2& newMousePosition) override {
    const glm::ivec2 delta = newMousePosition - mousePosition;
    this->mousePosition = newMousePosition;
    camera->handleInputRotate(glm::vec3(delta.y, delta.x, 0));
  }

private:
  // This follows the free camera.
  util::Frustum frustum;

  bool frustumCulling = true;

  std::shared_ptr<render::InputColorShader> colorShader;
  std::vector<std::unique_ptr<PointField>>  tiles;
};

int main(int argc, char **argv) {
  Demo12 demo;
  demo.run(argc, argv);

  return 0;
}
