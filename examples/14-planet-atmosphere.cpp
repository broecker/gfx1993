#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include "Atmosphere.h"
#include "DemoApp.h"
#include "Geometry.h"
#include "Pipeline.h"
#include "Shader.h"
#include "Camera.h"

using namespace gfx1993;
using namespace glm;

// A first prototype of rendering a planet's atmosphere as a real positioned
// volume around a placeholder sphere, viewed from orbit -- as opposed to
// 11-dynamic-sky.cpp's ground-level backdrop. See Atmosphere.h for the
// shared physics and PositionalAtmosphereFragmentShader's doc comment for
// what's different about the orbital case: the raymarch starts at the
// camera's actual position instead of a fixed ground point, and its alpha
// comes from the raymarch's own transmittance so it composites over the
// planet and space instead of assuming it's the whole background.
//
// The planet and sun are both placeholder spheres for now -- this demo
// exists to validate the atmosphere technique in isolation, before it needs
// to coexist with the (still unbuilt) cube-sphere quadtree terrain LOD.
// The planet is deliberately low-poly/flat-shaded to preview that eventual
// look.
//
// Scroll to zoom from ground-grazing to deep space; drag to orbit.
namespace {
  constexpr float kPlanetRadiusWorld = 10.f;

  // World-units-to-km scale: maps the placeholder planet's world-space
  // radius onto the atmosphere model's real (Earth-preset) bottomRadius, so
  // the shell's physical proportions come along for free. This makes the
  // glow at the limb quite thin (real atmospheres are paper-thin relative
  // to planet radius) -- see the project doc for the option of exaggerating
  // shell thickness for style, which would need bottomRadius/topRadius to
  // become shader parameters rather than fixed constants.
  constexpr float kKmPerWorldUnit = atmosphere::bottomRadius / kPlanetRadiusWorld;

  constexpr float kSunDistanceWorld = 55.f;
  constexpr float kSunRadiusWorld = 5.f;
}

// Simple Lambertian shading with a small ambient term so the night side
// isn't pure black, evoking faint starlight/earthshine.
class PlanetSurfaceShader : public FragmentShader {
public:
  Fragment shadeSingle(const ShadingGeometry &in) override {
    float ndotl = glm::max(dot(normalize(in.normal), sunDirection), 0.f);
    vec3 color = baseColor * (ambient + (1.f - ambient) * ndotl);

    Fragment out;
    out.color = vec4(color, 1.f);
    out.discard = false;
    return out;
  }

  vec3 sunDirection = vec3(0, 1, 0);
  vec3 baseColor = vec3(0.45f, 0.5f, 0.4f);
  float ambient = 0.05f;
};

class Demo14 : public DemoApp {
public:
  Demo14()
      : DemoApp("Demo 14 - Planet Atmosphere"),
        planet(kPlanetRadiusWorld, 10, 16),
        sun(kSunRadiusWorld, 8, 12),
        atmosphereQuad(vec4(1.f)),
        sunDirection(normalize(vec3(0.85f, 0.5f, -0.7f))) {}

protected:
  void init() override {
    planet.makeFlatShaded();

    surfaceShader = std::make_shared<PlanetSurfaceShader>();
    surfaceShader->sunDirection = sunDirection;

    sunShader = std::make_shared<SingleColorShader>(vec4(1.f, 0.95f, 0.85f, 1.f));

    skyboxVertShader = std::make_shared<SkyboxVertexShader>();
    atmosphereShader = std::make_shared<PositionalAtmosphereFragmentShader>();
    atmosphereShader->sunDirection = sunDirection;

    transform = std::make_shared<DefaultVertexTransform>();

    // The default orbit distance (30 units, DemoApp's default) shows the
    // whole scene but leaves the atmosphere shell only a couple of pixels
    // wide (it's proportioned to a physically thin 100km/6360km shell --
    // see kKmPerWorldUnit above). Start closer in so it's easier to spot.
    camera->handleInputTranslate(vec3(0, 0, -8.f));

    std::cout << "Placeholder planet + sun with a positioned, volumetric atmosphere.\n"
                 "Scroll to zoom from ground-grazing to deep space; drag to orbit."
              << std::endl;
  }

  void updateFrame(float dt) override {
    DemoApp::updateFrame(dt);
    totalTime += dt;
  }

  void renderFrame() override {
    renderConfig.clearBuffers(vec4(0.f, 0.f, 0.f, 1.f));

    transform->viewMatrix = camera->getViewMatrix();
    transform->projectionMatrix = camera->getProjectionMatrix();
    renderConfig.vertexShader = transform;
    renderConfig.depthWrite = true;
    renderConfig.depthTest = true;

    // Sun.
    transform->modelMatrix = glm::translate(sunDirection * kSunDistanceWorld);
    renderConfig.fragmentShader = sunShader;
    rasterizer->drawTriangles(renderConfig, sun.getVertices(), sun.getIndices());

    // Planet, slowly spinning for a bit of life.
    transform->modelMatrix = glm::rotate(totalTime * 0.05f, vec3(0, 1, 0));
    renderConfig.fragmentShader = surfaceShader;
    rasterizer->drawTriangles(renderConfig, planet.getVertices(), planet.getIndices());

    // Atmosphere, drawn last: a fullscreen quad using the same
    // reconstruct-the-view-ray trick as a regular skybox, but composited
    // over the planet+space using the depthbuffer we just drew into (so
    // nearer, opaque geometry above already wins the depth test) and alpha
    // from the raymarch's own transmittance rather than being treated as
    // opaque. See PositionalAtmosphereFragmentShader's doc comment.
    vec3 cameraWorldPos = vec3(glm::inverse(camera->getViewMatrix()) * vec4(0, 0, 0, 1));
    atmosphereShader->cameraPosition = cameraWorldPos * kKmPerWorldUnit;

    skyboxVertShader->inverseViewProjection =
        glm::inverse(camera->getProjectionMatrix() * camera->getViewMatrix());
    renderConfig.vertexShader = skyboxVertShader;
    renderConfig.fragmentShader = atmosphereShader;
    renderConfig.depthWrite = false;
    renderConfig.alphaBlending = true;
    rasterizer->drawTriangles(renderConfig, atmosphereQuad.getVertices(), atmosphereQuad.getIndices());
    renderConfig.alphaBlending = false;
  }

private:
  Sphere planet;
  Sphere sun;
  Quad atmosphereQuad;
  vec3 sunDirection;

  std::shared_ptr<DefaultVertexTransform> transform;
  std::shared_ptr<PlanetSurfaceShader> surfaceShader;
  std::shared_ptr<SingleColorShader> sunShader;
  std::shared_ptr<SkyboxVertexShader> skyboxVertShader;
  std::shared_ptr<PositionalAtmosphereFragmentShader> atmosphereShader;

  float totalTime = 0.f;
};

int main(int argc, char **argv) {
  Demo14 demo;
  demo.run(argc, argv);
  return 0;
}
