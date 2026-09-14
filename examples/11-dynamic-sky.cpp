#include <atomic>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>

#include "Atmosphere.h"
#include "DemoApp.h"
#include "Geometry.h"
#include "Pipeline.h"
#include "PlyGeometry.h"
#include "Profiler.h"
#include "Shader.h"
#include "Camera.h"
#include "StarField.h"
#include "Texture.h"

using namespace gfx1993;
using namespace glm;

// Physically based sky. See Atmosphere.h for the model itself (shared with
// 14-planet-atmosphere) and its source (Hillaire, EGSR 2020).
//
// Ray-marching the sky per pixel every frame turned out too slow, so instead
// this bakes it into a small equirectangular (lat/long) texture every few
// frames -- optionally on a background thread -- and has the skybox (and
// the bunny's reflection) look it up instead of ray-marching every pixel
// every frame. The sky's appearance only depends on view direction and the
// (slowly changing) sun direction, not on scene geometry or camera
// position, so baking it at low resolution and reusing it across several
// frames trades a small amount of visual fidelity for a large drop in
// per-frame cost.
//
// Press 'n' to compare the baked texture lookup against a live per-pixel
// raymarch directly.
namespace {
  constexpr unsigned int kSkyLatLongWidth = 128;
  constexpr unsigned int kSkyLatLongHeight = kSkyLatLongWidth / 2; // equirectangular, 2:1
  constexpr unsigned int kSkyBakeEveryNFrames = 8;

  // Stars are real world-space points, so they must stay within the
  // camera's projection far plane (see Camera.cpp's default projection,
  // far=100) or they get clipped away entirely.
  constexpr float kStarDistance = 50.f;
}

class Demo11 : public DemoApp {
public:
  Demo11() : DemoApp("Demo 11 - Dynamic Sky"), skybox(vec4(1.f)) {}

  ~Demo11() override {
    if (bakeThread.joinable()) {
      bakeThread.join();
    }
  }

protected:
  void init() override {
    fixedFunctionTransform = std::make_shared<DefaultVertexTransform>();
    gridShader = std::make_shared<InputColorShader>();

    skyboxVertShader = std::make_shared<SkyboxVertexShader>();
    atmosphereFragShader = std::make_shared<AtmosphereFragmentShader>();
    latLongSkyShader = std::make_shared<LatLongSkyboxFragmentShader>();
    reflectiveShader = std::make_shared<ReflectiveSkyboxFragmentShader>();
    reflectiveShader->exposure = atmosphereExposure;
    reflectiveShader->multiScatteringFactor = atmosphereMultiScatter;

    grid = std::make_unique<GridGeometry>();

    if (!bunny.loadPly("../models/bunny/reconstruction/bun_zipper_res3.ply")) {
      std::cerr << "Failed to load the Stanford bunny model." << std::endl;
    }
    bunny.transform = glm::scale(vec3(125.f));
    bunny.center();
    OrbitCamera* orbitCam = dynamic_cast<OrbitCamera*>(camera.get());
    orbitCam->setTarget(bunny.getCenter());
    // The default orientation looks dead level at the bunny, which puts the
    // horizon through screen center and shows mostly ground/grid. Tilt the
    // view up and around for a more natural 3/4 angle that also shows a
    // good amount of sky.
    orbitCam->handleInputRotate(glm::vec3(-25.f, 35.f, 0.f));

    starVertexTransform = std::make_shared<DefaultVertexTransform>();
    if (!stars.loadCatalog("../models/stars/bright_stars.csv")) {
      std::cerr << "Failed to load the star catalog." << std::endl;
    }

    // Bake once synchronously so the first frame isn't blank.
    vec3 initialSunDir = sunDirectionForDayTime(dayTime);
    bakeSkyTexture(initialSunDir);
    latLongSkyShader->texture = pendingSkyTexture;
    reflectiveShader->texture = pendingSkyTexture;
    stars.updateVisibleStars(dayTime * glm::two_pi<float>(), glm::radians(observerLatitudeDeg),
                             kStarDistance, starVisibilityForDayTime(dayTime));

    std::cout << "Press 'n' to toggle between the baked lat/long atmosphere and "
                 "the live per-pixel raymarch, "
                 "'p' to pause/resume the day/night cycle, "
                 "'[' / ']' to scrub time of day while paused, "
                 "',' / '.' to adjust observer latitude."
              << std::endl;
  }

  void updateFrame(float dt) override {
    DemoApp::updateFrame(dt);

    if (!daylightPaused) {
      dayTime += dt / dayLengthSeconds;
      dayTime -= floorf(dayTime);
    }

    vec3 sunDir = sunDirectionForDayTime(dayTime);
    atmosphereFragShader->sunDirection = sunDir;
    reflectiveShader->sunDirection = sunDir;
    reflectiveShader->useLiveAtmosphere = (skyMode == SkyMode::LiveAtmosphere);

    ++frameCounter;
    if (!bakeInFlight.load() && (frameCounter % kSkyBakeEveryNFrames == 0)) {
      if (bakeThread.joinable()) {
        bakeThread.join();
      }
      bakeInFlight = true;
      bakeThread = std::thread([this, sunDir] {
        GFX1993_SET_THREAD_NAME("gfx1993-sky-bake");
        bakeSkyTexture(sunDir);
        bakeInFlight = false;
      });

      // Stars are cheap (1000 trig evaluations) compared to the sky
      // raymarch, so this runs synchronously on the same cadence rather
      // than needing its own background thread.
      stars.updateVisibleStars(dayTime * glm::two_pi<float>(), glm::radians(observerLatitudeDeg),
                               kStarDistance, starVisibilityForDayTime(dayTime));
    }

    // Publish whatever the most recently finished bake produced. Cheap:
    // once per frame, not per pixel.
    {
      std::lock_guard<std::mutex> lock(skyTextureMutex);
      if (pendingSkyTexture) {
        latLongSkyShader->texture = pendingSkyTexture;
        reflectiveShader->texture = pendingSkyTexture;
      }
    }
  }

  void renderFrame() override {
    // Clear the buffers.
    {
      GFX1993_ZONE_N("app.clearBuffers");
      renderConfig.clearBuffers(glm::vec4(1, 0, 0, 1));
    }

    skyboxVertShader->inverseViewProjection = glm::inverse(camera->getProjectionMatrix() * camera->getViewMatrix());

    renderConfig.depthbuffer = nullptr;
    renderConfig.vertexShader = skyboxVertShader;
    renderConfig.fragmentShader = skyMode == SkyMode::LiveAtmosphere
        ? std::static_pointer_cast<FragmentShader>(atmosphereFragShader)
        : std::static_pointer_cast<FragmentShader>(latLongSkyShader);
    rasterizer->drawTriangles(renderConfig, skybox.getVertices(), skybox.getIndices());

    // Draw the starfield, still under the depthbuffer=nullptr regime set
    // above for the skybox -- stars are the most distant thing in the
    // scene, so painting them right after the sky and before everything
    // else keeps them a pure background layer. A rotation-only view matrix
    // keeps them from shifting with camera translation (they're at
    // infinity); gridShader is a stateless InputColorShader, reused here
    // since each star's final display color is already baked into its
    // vertex color by StarField::updateVisibleStars.
    starVertexTransform->modelMatrix = glm::mat4(1.f);
    starVertexTransform->viewMatrix = glm::mat4(glm::mat3(camera->getViewMatrix()));
    starVertexTransform->projectionMatrix = camera->getProjectionMatrix();
    renderConfig.vertexShader = starVertexTransform;
    renderConfig.fragmentShader = gridShader;
    renderConfig.pointSize = 1;
    rasterizer->drawPoints(renderConfig, stars.getVertices(), stars.getIndices());

    // Reset the render matrices.
    fixedFunctionTransform->modelMatrix = glm::mat4(1.f);
    fixedFunctionTransform->viewMatrix = camera->getViewMatrix();
    fixedFunctionTransform->projectionMatrix = camera->getProjectionMatrix();

    // Draw the floor grid.
    renderConfig.depthWrite = true;
    renderConfig.depthTest = true;
    renderConfig.vertexShader = fixedFunctionTransform;
    renderConfig.fragmentShader = gridShader;
    rasterizer->drawLines(renderConfig, grid->getVertices(),
                          grid->getIndices());

    // Draw the bunny, reflecting the current sky off its surface.
    reflectiveShader->eyePosition = vec3(glm::inverse(camera->getViewMatrix()) * vec4(0.f, 0.f, 0.f, 1.f));
    fixedFunctionTransform->modelMatrix = bunny.transform;
    renderConfig.fragmentShader = reflectiveShader;
    rasterizer->drawTriangles(renderConfig, bunny.getVertices(), bunny.getIndices());
  }

  void handleKeyboard(unsigned char key, const glm::ivec2& mouse) override {
    if (key == 'n') {
      skyMode = skyMode == SkyMode::BakedAtmosphere ? SkyMode::LiveAtmosphere : SkyMode::BakedAtmosphere;
      std::cout << "Sky shader: "
                << (skyMode == SkyMode::BakedAtmosphere ? "baked lat/long atmosphere" : "live per-pixel atmosphere")
                << std::endl;
    }

    if (key == 'p') {
      daylightPaused = !daylightPaused;
      std::cout << "Day/night cycle " << (daylightPaused ? "paused" : "resumed") << std::endl;
    }

    if (key == '[') {
      daylightPaused = true;
      dayTime -= 0.01f;
      dayTime -= floorf(dayTime);
    }
    if (key == ']') {
      daylightPaused = true;
      dayTime += 0.01f;
      dayTime -= floorf(dayTime);
    }

    if (key == ',') {
      observerLatitudeDeg = glm::clamp(observerLatitudeDeg - 5.f, -90.f, 90.f);
      std::cout << "Observer latitude: " << observerLatitudeDeg << " deg" << std::endl;
    }
    if (key == '.') {
      observerLatitudeDeg = glm::clamp(observerLatitudeDeg + 5.f, -90.f, 90.f);
      std::cout << "Observer latitude: " << observerLatitudeDeg << " deg" << std::endl;
    }
  }

private:
  // dayTime: 0 = midnight, 0.25 = sunrise, 0.5 = noon, 0.75 = sunset.
  float sunElevationForDayTime(float t) const {
    return sinf(glm::two_pi<float>() * (t - 0.25f)) * glm::half_pi<float>();
  }

  vec3 sunDirectionForDayTime(float t) const {
    float elevation = sunElevationForDayTime(t);
    return vec3(sinf(sunAzimuth) * cosf(elevation), sinf(elevation), cosf(sunAzimuth) * cosf(elevation));
  }

  // Stars fade in once the sun dips below the horizon, reaching full
  // brightness at the end of astronomical twilight (-18 degrees).
  float starVisibilityForDayTime(float t) const {
    float elevationDeg = glm::degrees(sunElevationForDayTime(t));
    return glm::clamp((-2.f - elevationDeg) / (18.f - 2.f), 0.f, 1.f);
  }

  // Bakes the sky into a kSkyLatLongWidth x kSkyLatLongHeight equirectangular
  // texture for the given sun direction, and publishes it to
  // pendingSkyTexture under skyTextureMutex. May run on a background thread;
  // touches no state besides its arguments, compile-time constants, and the
  // write-once-at-init atmosphereExposure/atmosphereMultiScatter floats.
  void bakeSkyTexture(const vec3 &sunDir) {
    GFX1993_ZONE_N("sky.bake");

    std::shared_ptr<Texture> newTexture(bakeSkyLatLongTexture(
        atmosphere::groundLevelPosition(), sunDir, atmosphereExposure, atmosphereMultiScatter,
        kSkyLatLongWidth, kSkyLatLongHeight));

    std::lock_guard<std::mutex> lock(skyTextureMutex);
    pendingSkyTexture = newTexture;
  }

  std::unique_ptr<GridGeometry> grid;
  PlyGeometry bunny;
  StarField stars;
  float observerLatitudeDeg = 45.f; // northern hemisphere default

  std::shared_ptr<DefaultVertexTransform> fixedFunctionTransform;
  std::shared_ptr<DefaultVertexTransform> starVertexTransform;
  std::shared_ptr<FragmentShader> gridShader;

  std::shared_ptr<SkyboxVertexShader> skyboxVertShader;
  std::shared_ptr<AtmosphereFragmentShader> atmosphereFragShader;
  std::shared_ptr<LatLongSkyboxFragmentShader> latLongSkyShader;
  std::shared_ptr<ReflectiveSkyboxFragmentShader> reflectiveShader;

  enum class SkyMode { BakedAtmosphere, LiveAtmosphere };
  SkyMode skyMode = SkyMode::BakedAtmosphere;

  std::mutex skyTextureMutex;
  std::shared_ptr<Texture> pendingSkyTexture; // guarded by skyTextureMutex
  std::thread bakeThread;
  std::atomic<bool> bakeInFlight{false};
  unsigned int frameCounter = 0;

  // Fixed atmosphere tuning parameters, set once and never mutated -- safe
  // for the bake thread to read without synchronization.
  float atmosphereExposure = 12.f;
  float atmosphereMultiScatter = 0.5f;

  Quad skybox;

  // Time of day, in [0,1); see updateFrame().
  float dayTime = 0.4f;
  float dayLengthSeconds = 60.f;
  bool daylightPaused = false;
  // Fixed compass direction the sun arcs through, in radians.
  float sunAzimuth = 0.6f;
};

int main(int argc, char **argv) {
  Demo11 demo;
  demo.run(argc, argv);

  return 0;
}
