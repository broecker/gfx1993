#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "DemoApp.h"
#include "geometry/GridGeometry.h"
#include "geometry/PlyGeometry.h"
#include "geometry/Quad.h"
#include "base/Pipeline.h"
#include "rendering/Shader.h"

using namespace gfx1993;
using namespace glm;


struct PointLight {
  glm::vec3 position;
  glm::vec3 color;
};

// Calculate lighting in the VertexShader based on surface normals. No need for
// interpolation in the fragment shader, as the results are constant.
class FlatVertexShader : public render::DefaultVertexTransform {
public:
  render::VertexOut transformSingle(const render::Vertex &in) override {
    render::VertexOut out = DefaultVertexTransform::transformSingle(in);

    // TODO(mbroecker) -- calculate lighting.
    vec3 ambient = vec3(0.1f);

    vec3 L = normalize(light.position - vec3(in.position));

    vec3 diffuse = surfaceColor * dot(L, in.normal) * light.color;
    vec3 specular(0);

    out.color = glm::vec4(ambient + diffuse + specular, 1.0f);

    return out;
  }

  PointLight light;
  glm::vec3 surfaceColor;

};

// Calculate lighting in the VertexShader, interpolate results in the fragment
// shader.
class GoroudVertexShader : public render::DefaultVertexTransform {

};

class GoroudFragmentShader : public render::FragmentShader {
public:
  render::Fragment shadeSingle(const render::ShadingGeometry &in) override {
    render::Fragment frag;

    frag.color = in.color;
    frag.discard = false;

    return frag;
  }
};

// Calculate lighting in the Fragment shader, by using the transformed world
// position and normals.
class BlinnPhongShader : public render::FragmentShader {
public:
  render::Fragment shadeSingle(const render::ShadingGeometry& in) override {
    render::Fragment out;
    vec3 ambient(0.1f);
    vec3 diffuse(0);
    vec3 specular(0);

    vec3 L = normalize(light.position - vec3(in.position));

    float l = dot(L, in.surfaceNormal);
    if (l > 0) {
      diffuse = surfaceColor * light.color;
    }

    out.color = glm::vec4(ambient + diffuse + specular, 1.0f);

    return out;
  }

  PointLight light;
  glm::vec3 surfaceColor;
};


class Demo09 : public DemoApp {
public:
  Demo09() : DemoApp("Demo 09 - Shading") {}

protected:
  void init() override {
    gridShader = std::make_shared<render::InputColorShader>();
    phongShader = std::make_shared<BlinnPhongShader>(); 
    phongShader->light.color = vec3(0.7);
    phongShader->light.position = vec3(50.f, 50.f, 20.f);
    phongShader->surfaceColor = vec3(0.7);

    renderConfig.vertexShader =
        std::make_shared<render::DefaultVertexTransform>();
    renderConfig.fragmentShader = std::make_shared<render::NormalColorShader>();

    assert(bunny.loadPly("../models/bunny/reconstruction/bun_zipper_res3.ply"));
    bunny.transform = glm::scale(glm::vec3(125.f));
    bunny.center();

    dynamic_cast<util::OrbitCamera*>(camera.get())->setTarget(bunny.getCenter());
  }

  void renderFrame() override {
    // reset the render matrices
    render::DefaultVertexTransform *dvt =
        dynamic_cast<render::DefaultVertexTransform *>(
            renderConfig.vertexShader.get());
    dvt->modelMatrix = glm::mat4(1.f);
    dvt->viewMatrix = camera->getViewMatrix();
    dvt->projectionMatrix = camera->getProjectionMatrix();

    // Clear the buffers
    renderConfig.clearBuffers(glm::vec4(0.7f, 0.7f, 0.9f, 1));

    // Draw all the bunny; first draw it as triangles, without a color buffer.
    // This will still fill the depth buffer.
    renderConfig.fragmentShader = phongShader;
    dvt->modelMatrix = bunny.transform;
    rasterizer->drawTriangles(renderConfig, bunny.getVertices(),
                              bunny.getIndices());


    // Finally, draw the grid. We want to draw it last, so the bunny's
    // depth pass will block any grid lines behind the model.
    if (drawGrid) {
      renderConfig.fragmentShader = gridShader;
      dvt->modelMatrix = glm::mat4(1);
      rasterizer->drawLines(renderConfig, 
                            grid.getVertices(),
                            grid.getIndices());
    }
  }

  void handleKeyboard(unsigned char key, const glm::ivec2& mousePosition) override {
    if (key == 'b') {
      renderConfig.drawTriangleBounds = !renderConfig.drawTriangleBounds;
      std::cout << "Drawing tri raster bounds: " << renderConfig.drawTriangleBounds << std::endl;
    }

    if (key == 'c') {
      renderConfig.cullBackFaces = !renderConfig.cullBackFaces;
      std::cout << "Culling backfaces: " << renderConfig.cullBackFaces << std::endl;
    }

    if (key == 'g') {
      drawGrid = !drawGrid;
    }
  }

private:
  geometry::GridGeometry grid;
  geometry::PlyGeometry bunny;

  std::shared_ptr<render::FragmentShader> gridShader;
  std::shared_ptr<BlinnPhongShader> phongShader;

  PointLight light;

  bool drawGrid = true;
};

int main(int argc, char **argv) {
  Demo09 demo;
  demo.run(argc, argv);

  return 0;
}
