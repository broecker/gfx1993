#include <cstdlib>
#include <iostream>
#include <memory>
#include <vector>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>

#include "DemoApp.h"
#include "GridGeometry.h"
#include "Quad.h"
#include "Terrain.h"
#include "Pipeline.h"
#include "Shader.h"
#include "Texture.h"
#include "BoundingVolumes.h"
#include "Camera.h"
#include "Frustum.h"

using namespace gfx1993;
using namespace glm;

constexpr unsigned int heightmapRes = 128;

class HeightmapShader : public FragmentShader {
public:
  HeightmapShader(std::shared_ptr<HeightMap> tex) : heightmap(tex) {}

  Fragment shadeSingle(const ShadingGeometry& sgeo) {
    Fragment frag;
    frag.discard = false;

    float h = heightmap->getTexel(sgeo.texcoord);
    float d = heightmap->getMaxHeight() - heightmap->getMinHeight();

    h = (h - heightmap->getMinHeight()) / d;
    vec3 color = mix(min, max, h);
    frag.color = vec4(color, 1.f);
    return frag;
  }

private:
  vec3 min = vec3(0.1f), max = vec3(1.f);
  std::shared_ptr<HeightMap>  heightmap;
};

class Demo13 : public DemoApp {
public:
  Demo13() : DemoApp("Demo 13 - Voxelspace"), frustum(mat4(1.f), mat4(1.f)), quad(Quad::makeXZQuad(glm::vec2(heightmapRes, heightmapRes))) {
      camera = std::make_unique<FreeCamera>(
          glm::perspective(30.f, static_cast<float>(width) / height, 1.f, 200.f), 
          vec3(0, -2, 10));(vec2(20.f, 0.f));

      frustum = Frustum(camera->getProjectionMatrix(), camera->getViewMatrix());
  }

protected:
  void init() override {
    inputShader = std::make_shared<InputColorShader>();
    renderConfig.vertexShader =
        std::make_shared<DefaultVertexTransform>();
    renderConfig.fragmentShader = colorShader;

    colorShader = std::make_shared<SingleColorShader>(glm::vec4(1,0,1,1));

    heightMap = HeightMap::perlinNoise(256, 256, vec3(1, 20.f, 1));
    points = std::make_unique<PointField>(heightMap);

    hmShader = std::make_shared<HeightmapShader>(heightMap);

    quad.setRandomVertexColors();

    logFrameTime = false;
    updateFrame(0);
  }

  void updateFrame(float dt) override {
    camera->update(dt);
    frustum.update(camera->getViewMatrix());
  }

  void renderOverhead() {
    // reset the render matrices
    DefaultVertexTransform *dvt =
        dynamic_cast<DefaultVertexTransform *>(
            renderConfig.vertexShader.get());

    dvt->viewMatrix = glm::lookAt(vec3(0,1,0), vec3(0), vec3(0,0,-1));

    float border = heightmapRes + 100.f;
    dvt->projectionMatrix = glm::ortho<float>(
      -border, border, -border, border, -1, 1);

    renderConfig.depthTest = false;
    renderConfig.depthWrite = false;
    renderConfig.fragmentShader = hmShader;
    float size = heightmapRes;
    rasterizer->drawTriangles(renderConfig,
      {
        Vertex(vec4(-size, 0, -size,1), vec3(0,1,0), vec4(1), vec2(0,1)),
        Vertex(vec4(-size, 0,  size,1), vec3(0,1,0), vec4(1), vec2(0,0)),
        Vertex(vec4( size, 0,  size,1), vec3(0,1,0), vec4(1), vec2(1,0)),
        Vertex(vec4( size, 0, -size,1), vec3(0,1,0), vec4(1), vec2(1,1))},
      {0,1,2, 2,3,0}
    );

    // Calculate and render view frustum.


    glm::mat4 viewProjection = camera->getProjectionMatrix() * camera->getViewMatrix();
    glm::mat4 ivp = inverse(viewProjection);

    vec4 eyePosition = camera->getViewMatrix() * vec4(0,0,0,1);

    vec4 farLeftFrustum = ivp * vec4(-1,0,1,1);
    farLeftFrustum /= farLeftFrustum.w;
    vec4 farRightFrustum = ivp * vec4(1,0,1,1);
    farRightFrustum /= farRightFrustum.w;

    renderConfig.fragmentShader = colorShader;
    
    VertexList frustum = {
      Vertex(eyePosition),
      Vertex(farLeftFrustum),
      Vertex(farRightFrustum)
    };
    for (auto& v : frustum) {
      v.position.y = 0.f;
    }

    rasterizer->drawLines(renderConfig, frustum, {0,1, 0,2, 1,2} );

    VertexList slices;
    IndexList sliceIndices;

    float steps = 5;
    for (int slice = steps; slice > 0; --slice) {
      // TODO: This should not get closer than the frustum's near
      float depth = static_cast<float>(slice) / steps;

      vec4 left = ivp * vec4(-1,0,depth,1);
      left /= left.w;
      vec4 right = ivp * vec4(1,0,depth,1);
      right /= right.w;

      
      Vertex l(left);
      l.position.y = 0.f;

      Vertex r(right);
      r.position.y = 0.f;

      slices.push_back(l);
      slices.push_back(r);
      sliceIndices.push_back(slices.size()-2);
      sliceIndices.push_back(slices.size()-1);
    }


    rasterizer->drawLines(renderConfig, slices, sliceIndices);

  }

  void renderScene() {
    // reset the render matrices
    DefaultVertexTransform *dvt =
        dynamic_cast<DefaultVertexTransform *>(
            renderConfig.vertexShader.get());

    dvt->projectionMatrix = camera->getProjectionMatrix();
    dvt->viewMatrix = camera->getViewMatrix();  
    dvt->modelMatrix = glm::mat4(1.f);
    renderConfig.fragmentShader = inputShader;
    renderConfig.pointSize = 5;

    renderConfig.depthTest = true;
    renderConfig.depthWrite = true;

    // Draw the heightmap texture.
    renderConfig.fragmentShader = hmShader;
    rasterizer->drawTriangles(renderConfig, quad.getVertices(), quad.getIndices());

    renderConfig.depthTest = false;
    renderConfig.depthWrite = false;
    rasterizer->drawLines(renderConfig, grid.getVertices(), grid.getIndices());

    // rasterizer->drawVoxelSpace(
    //     renderConfig,
    //     *heightMap.get(),
    //     camera->getViewMatrix(),
    //     camera->getProjectionMatrix());
  }


  void renderFrame() override {
    // Clear the buffers
    renderConfig.clearBuffers(glm::vec4(0.7f, 0.7f, 0.9f, 1));

    if (drawOverhead) {
      renderOverhead();
    } else {
      renderScene();
    }

    // auto& debugInfo = rasterizer->getDebugInfo();

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
      case 'o':
        drawOverhead = !drawOverhead;
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
    // Voxelspace landscape cannot handle pitch (yet). Let's just update yaw.
    camera->handleInputRotate(glm::vec3(0, delta.x, 0));
  }

private:
  // This follows the free camera.
  Frustum frustum;

  bool frustumCulling = true;

  bool drawOverhead = false;

  std::shared_ptr<HeightmapShader>            hmShader;
  std::shared_ptr<InputColorShader>   inputShader;
  std::shared_ptr<SingleColorShader>  colorShader;

  std::shared_ptr<HeightMap>          heightMap;
  std::unique_ptr<PointField>       points;

  GridGeometry                      grid;
  Quad                              quad;

};

int main(int argc, char **argv) {
  Demo13 demo;
  demo.run(argc, argv);

  return 0;
}
