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
  // In world-space coordinates.
  vec3 position;
  vec3 color;

  // Constant, linear, quadratic attenuation; see
  // Van Verth, Bishop: Essential Mathematics for Games, 2nd Ed, pg 326
  // k_c, k_l, k_q
  vec3 attenuation;

  float intensity;
};

// Calculate lighting in the VertexShader based on surface normals. No need for
// interpolation in the fragment shader, as the results are constant.
class FlatVertexShader : public render::DefaultVertexTransform {
public:
  render::VertexOut transformSingle(const render::Vertex &in) override {
    render::VertexOut out = DefaultVertexTransform::transformSingle(in);

    return out;
  }

  PointLight light;
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

class PhongShader : public render::FragmentShader {
public:
  render::Fragment shadeSingle(const render::ShadingGeometry& in) override {
    render::Fragment out;
    assert(profile != nullptr);
    auto p = profile->startTiming("rasterize.tris.shade.phong");

    vec3 L = light.position - vec3(in.position);
    float lightDist = length(L);
    L = L / lightDist;

    // Calculate light intensity.
    float distanceAttenuation = dot(light.attenuation, vec3(1.f, lightDist, lightDist*lightDist));
    float iL = light.intensity / distanceAttenuation;

    // Light model components.
    vec3 ambient = iL * light.color * surfaceColor;
    vec3 diffuse = iL * light.color * surfaceColor * glm::max(glm::dot(in.normal, L), 0.f);

   
    vec3 specular(0);
    if (dot(in.normal, L) < 0) { 
      vec3 view = in.position - vec3(eyePosition);
      vec3 half = (L + view) / glm::length(L + view);

      specular = iL * light.color * surfaceColor * pow(max(0.f, dot(in.normal, half)), specularExponent);
    }

    out.color = glm::vec4(ambient + diffuse + specular, 1.0f);

    // TODO: scale or tone-map?
    out.color = glm::min(out.color, vec4(1.0));
    return out;
  }

  PointLight light;
  vec3 surfaceColor;
  vec3 emission = vec3(0);
  float specularExponent;

  vec3 eyePosition;

  util::RenderProfile* profile = nullptr;
};

class Demo09 : public DemoApp {
public:
  Demo09() : DemoApp("Demo 09 - Shading") {}

protected:
  void init() override {
    light.color = vec3(0.5);
    light.position = vec3(0.f, 50.f, 0.f);
    light.intensity = 0.8f;
    light.attenuation = vec3(0.6f, 0.4f, 0.1f);

    gridShader = std::make_shared<render::InputColorShader>();
    phongShader = std::make_shared<PhongShader>(); 
    phongShader->light = light;
    phongShader->surfaceColor = vec3(0.7, 0.4, 0.2);
    phongShader->specularExponent = 6.f;

    renderConfig.vertexShader =
        std::make_shared<render::DefaultVertexTransform>();
    
    assert(bunny.loadPly("../models/bunny/reconstruction/bun_zipper_res3.ply"));
    bunny.transform = glm::scale(glm::vec3(125.f));
    bunny.center();

    // Randomize vertex colors;
    for (render::Vertex& v : bunny.getMutableVertexList()) {
      v.color.r = static_cast<float>(rand()) / RAND_MAX;
      v.color.g = static_cast<float>(rand()) / RAND_MAX;
      v.color.b = 1.f - v.color.r - v.color.g;
    } 

    dynamic_cast<util::OrbitCamera*>(camera.get())->setTarget(bunny.getCenter());
  }

  void updateFrame(float dt) override {
    DemoApp::updateFrame(dt);

    lightTheta += (dt * 5.f);
    light.position.x = sin(lightTheta) * lightRadius;
    light.position.y = lightRadius;
    light.position.z = cos(lightTheta) * lightRadius;
    
    phongShader->light.position = light.position;
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

    
    phongShader->eyePosition = inverse(dvt->viewMatrix) * vec4(0,0,0,1);
    phongShader->profile = &rasterizer->getProfile();

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
  std::shared_ptr<PhongShader> phongShader;

  PointLight light;
  // For animation.
  float lightRadius = 30.f;
  float lightTheta = 0.f;


  bool drawGrid = true;
};

int main(int argc, char **argv) {
  Demo09 demo;
  demo.run(argc, argv);

  return 0;
}
