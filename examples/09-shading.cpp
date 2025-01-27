#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "DemoApp.h"
#include "CubeGeometry.h"
#include "GridGeometry.h"
#include "PlyGeometry.h"
#include "Quad.h"
#include "Sphere.h"
#include "Pipeline.h"
#include "Shader.h"

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

  vec3 ambient;

  float intensity;
};

struct Material {
  vec3 color;
  vec3 emission = vec3(0.f);
  float specularExponent;
};

// General rendering equation; however, it's calculated in world coordinates,
// not eye coordinates.
vec3 shade(const PointLight& light, 
           const Material& surface,
           const vec3& worldPosition,
           const vec3& worldNormal,
           const vec3& eyePosition) {
 vec3 L = light.position - vec3(worldPosition);
    float lightDist = length(L);
    L = L / lightDist;

    // Calculate light intensity.
    float distanceAttenuation = dot(light.attenuation, vec3(1.f, lightDist, lightDist*lightDist));
    float iL = light.intensity / distanceAttenuation;

    // Light model components.
    vec3 ambient = iL * light.ambient * surface.color;
    vec3 diffuse = iL * light.color * surface.color * glm::max(glm::dot(worldNormal, L), 0.f);

    vec3 specular(0);
    if (dot(worldNormal, L) < 0) { 
      vec3 view = vec3(worldPosition) - vec3(eyePosition);
      vec3 half = (L + view) / glm::length(L + view);

      specular = iL * light.color * surface.color * pow(max(0.f, dot(worldNormal, half)), surface.specularExponent);
    }

    vec3 color = surface.emission + ambient + diffuse + specular;

    // TODO: scale or tone-map?
    color = glm::min(color, vec3(1.0));

    return color;
}

class GoraudVertexShader : public render::DefaultVertexTransform {
public:
  render::VertexOut transformSingle(const render::Vertex &in) override {
    assert(profile != nullptr);
    auto p = profile->startTiming("rasterize.tris.shade.goraud");

    render::VertexOut out = DefaultVertexTransform::transformSingle(in);
    
    vec3 color = shade(
      light,
      surface,
      out.worldPosition,
      out.worldNormal,
      eyePosition
    );

    out.color = vec4(color, 1.0);
    return out;
  }

  PointLight  light;
  Material    surface;
  vec3 eyePosition;

  util::RenderProfile* profile = nullptr;
};

class PhongShader : public render::FragmentShader {
public:
  render::Fragment shadeSingle(const render::ShadingGeometry& in) override {
    render::Fragment out;
    assert(profile != nullptr);
    auto p = profile->startTiming("rasterize.tris.shade.phong");

    out.color = vec4(shade(light, surface, in.position, in.normal, eyePosition), 1.0);

    return out;
  }

  PointLight  light;
  Material    surface;
  vec3        eyePosition;

  util::RenderProfile* profile = nullptr;
};

class Demo09 : public DemoApp {
public:
  Demo09() : DemoApp("Demo 09 - Shading"), cube(geometry::Cube::makeSolid(vec3(2.5))) {}

protected:
  void init() override {
    light.color = vec3(0.5);
    light.position = vec3(0.f, 50.f, 0.f);
    light.intensity = 500.f;
    light.attenuation = vec3(0.6f, 0.4f, 0.1f);
    light.ambient = vec3(0.07, 0.07, 0.09);

    Material surface {.color = vec3(0.7, 0.5, 0.2), 
                      .emission=vec3(0),
                      .specularExponent=2.f};

    fixedFunctionShader = std::make_shared<render::DefaultVertexTransform>();
    inputColorShader = std::make_shared<render::InputColorShader>();
    phongShader = std::make_shared<PhongShader>(); 
    phongShader->light = light;
    phongShader->surface = surface;

    goraudShader = std::make_shared<GoraudVertexShader>();
    goraudShader->light = light;
    goraudShader->surface = surface;

    colorShader = std::make_shared<render::SingleColorShader>(vec4(1));
    renderConfig.vertexShader =
        std::make_shared<render::DefaultVertexTransform>();
    
    geometry = std::make_shared<geometry::Sphere>(15.f, 9, 
    18);
    geometry->setRandomFaceColors();
    flatGeometry = std::make_shared<geometry::Sphere>(15.f, 9, 18);
    flatGeometry->makeFlatShaded();
    flatGeometry->setRandomFaceColors();
  }

  void updateFrame(float dt) override {
    DemoApp::updateFrame(dt);

    lightTheta += (dt * 5.f);
    light.position.x = sin(lightTheta) * lightRadius;
    light.position.y = lightRadius;
    light.position.z = cos(lightTheta) * lightRadius;
    
    phongShader->light = light;
    goraudShader->light = light;

    cube.transform = glm::translate(light.position);
  }

  void renderFrame() override {
    // Reset the render matrices.
    fixedFunctionShader->modelMatrix = glm::mat4(1.f);
    fixedFunctionShader->viewMatrix = camera->getViewMatrix();
    fixedFunctionShader->projectionMatrix = camera->getProjectionMatrix();

    goraudShader->modelMatrix = glm::mat4(1.f);
    goraudShader->viewMatrix = camera->getViewMatrix();
    goraudShader->projectionMatrix = camera->getProjectionMatrix();

    // Clear the buffers
    renderConfig.clearBuffers(glm::vec4(0.7f, 0.7f, 0.9f, 1));
    
    // Set up shader uniforms.
    glm::vec4 eyePosition = inverse(fixedFunctionShader->viewMatrix) * vec4(0,0,0,1);
    phongShader->eyePosition = eyePosition;
    phongShader->profile = &rasterizer->getProfile();
    goraudShader->eyePosition = eyePosition;
    goraudShader->profile = &rasterizer->getProfile();

    // Here is the difference in shading: 
    // The Phong shading model is per fragment; we use the default vertex 
    // transform, which also calculates per-fragment position and normal
    // information which we'll use during shading.
    // The Goraud shader, on the other hand, calculates the lighting information
    // per vertex and writes out a color value per vertex, which is then
    // interpolated by the rasterizer and the fragment shader.

    // Flat shading
    renderConfig.vertexShader = goraudShader;
    goraudShader->modelMatrix = glm::translate(vec3(-40, 0, 0));
    renderConfig.fragmentShader = inputColorShader;
    rasterizer->drawTriangles(renderConfig, 
                              flatGeometry->getVertices(),
                                flatGeometry->getIndices());
    
    // Goraud shading
    renderConfig.vertexShader = goraudShader;
    goraudShader->modelMatrix = glm::mat4(1.f);
    renderConfig.fragmentShader = inputColorShader;
    rasterizer->drawTriangles(renderConfig, 
                              geometry->getVertices(),
                              geometry->getIndices());

    // Phong shading
    renderConfig.vertexShader = fixedFunctionShader;
    fixedFunctionShader->modelMatrix = glm::translate(vec3(40, 0, 0));
    renderConfig.fragmentShader = phongShader;
    rasterizer->drawTriangles(renderConfig, 
                              geometry->getVertices(),
                               geometry->getIndices());

    // Draw a small cube where the light is.
    fixedFunctionShader->modelMatrix = cube.transform;
    renderConfig.vertexShader = fixedFunctionShader;
    renderConfig.fragmentShader = colorShader;
    colorShader->setColor(vec4(light.color, 1));
    rasterizer->drawTriangles(renderConfig, cube.getVertices(), cube.getIndices());

    // Finally, draw the grid. We want to draw it last, so the bunny's
    // depth pass will block any grid lines behind the model.
    if (drawGrid) {
      renderConfig.fragmentShader = inputColorShader;
      fixedFunctionShader->modelMatrix = glm::mat4(1);
      renderConfig.vertexShader = fixedFunctionShader;
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

    if (key == 'g') {
      drawGrid = !drawGrid;
    }
  }

private:
  geometry::GridGeometry grid;

  std::shared_ptr<geometry::Geometry> geometry;
  std::shared_ptr<geometry::Geometry> flatGeometry;

  geometry::Cube cube;

  std::shared_ptr<render::DefaultVertexTransform> fixedFunctionShader;
  std::shared_ptr<render::FragmentShader> inputColorShader;
  std::shared_ptr<PhongShader> phongShader;
  std::shared_ptr<render::SingleColorShader> colorShader;
  std::shared_ptr<GoraudVertexShader> goraudShader;
  
  PointLight light;
  // For animation.
  float lightRadius = 50.f;
  float lightTheta = 0.f;

  bool drawGrid = true;
};

int main(int argc, char **argv) {
  Demo09 demo;
  demo.run(argc, argv);

  return 0;
}
