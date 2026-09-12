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

// Physically based sky, based on:
//   Sebastien Hillaire, "A Scalable and Production Ready Sky and Atmosphere
//   Rendering Technique", EGSR 2020.
//   https://sebh.github.io/publications/egsr2020.pdf
//
// The paper's real-time trick is to precompute a transmittance LUT, a
// multi-scattering LUT, and a sky-view LUT on the GPU so the actual sky
// pixel shader is just a couple of texture fetches. gfx1993 is a CPU
// software rasterizer with no compute-shader/texture-write stage, so this
// instead ray-marches the single-scattering integral directly (the paper's
// core physical model, Section 4). Multiple scattering (Section 5.2's
// precomputed LUT, built from an iterative dual-scattering approximation)
// is out of scope for a per-pixel shader with no prepass, so it's replaced
// with a simple ambient fudge term (see multiScatteringFactor below).
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

// The physics itself, factored into free functions that take every input
// explicitly. This makes it safe to call from a background bake thread: it
// touches no shared mutable state, only its by-value arguments and
// compile-time constants.
namespace atmosphere {

struct Medium {
  vec3 scatteringRayleigh;
  vec3 scatteringMie;
  vec3 extinction;
};

// All distances in kilometers, matching the paper's own units and its
// reference implementation's Earth preset
// (https://github.com/sebh/UnrealEngineSkyAtmosphere).
constexpr float bottomRadius = 6360.f;
constexpr float topRadius = 6460.f;
constexpr float rayleighScaleHeight = 8.f;
constexpr float mieScaleHeight = 1.2f;
constexpr float mieG = 0.8f;
// A fixed, token "eye level" viewer height above the ground. Since this is a
// skybox rendered independent of the scene's own tiny local camera
// position/units, the sky's appearance only depends on this height and the
// view/sun angles.
constexpr float eyeHeightKm = 0.2f;

constexpr vec3 rayleighScattering = vec3(0.005802f, 0.013558f, 0.033100f); // 1/km
constexpr vec3 mieScattering = vec3(0.003996f);                            // 1/km
constexpr vec3 mieExtinction = vec3(0.004440f);                            // 1/km
constexpr vec3 ozoneAbsorption = vec3(0.000650f, 0.001881f, 0.000085f);    // 1/km

constexpr vec3 sunIlluminance = vec3(1.f);
constexpr float sunAngularRadius = 0.004675f; // radians
constexpr float sunDiskLuminance = 4000.f;

inline float raySphereIntersectNearest(const vec3 &rayOrigin, const vec3 &rayDir,
                                       const vec3 &sphereCenter, float sphereRadius) {
  vec3 oc = rayOrigin - sphereCenter;
  float b = dot(oc, rayDir);
  float c = dot(oc, oc) - sphereRadius * sphereRadius;
  float discriminant = b * b - c;
  if (discriminant < 0.f) {
    return -1.f;
  }
  float sqrtD = sqrtf(discriminant);
  float t0 = -b - sqrtD;
  float t1 = -b + sqrtD;
  if (t0 >= 0.f) {
    return t0;
  }
  if (t1 >= 0.f) {
    return t1;
  }
  return -1.f;
}

inline float rayleighPhase(float cosTheta) {
  const float k = 3.f / (16.f * glm::pi<float>());
  return k * (1.f + cosTheta * cosTheta);
}

// Cornette-Shanks approximation of the Mie phase function.
inline float miePhase(float g, float cosTheta) {
  const float k = 3.f / (8.f * glm::pi<float>()) * (1.f - g * g) / (2.f + g * g);
  return k * (1.f + cosTheta * cosTheta) /
        powf(1.f + g * g - 2.f * g * cosTheta, 1.5f);
}

// Rayleigh/Mie density falls off exponentially with height; ozone forms a
// tent-shaped layer peaking around 25km (both pieces evaluate to 1 there).
inline Medium sampleMedium(float height) {
  float densityRayleigh = expf(-height / rayleighScaleHeight);
  float densityMie = expf(-height / mieScaleHeight);
  float densityOzone = glm::clamp(
      height < 25.f ? (height / 15.f - 2.f / 3.f) : (-height / 15.f + 8.f / 3.f),
      0.f, 1.f);

  Medium m;
  m.scatteringRayleigh = rayleighScattering * densityRayleigh;
  m.scatteringMie = mieScattering * densityMie;
  vec3 extinctionOzone = ozoneAbsorption * densityOzone;
  m.extinction = m.scatteringRayleigh + (mieExtinction * densityMie) + extinctionOzone;
  return m;
}

inline bool inEarthShadow(const vec3 &P, const vec3 &sunDir) {
  const vec3 planetCenter(0.f);
  return raySphereIntersectNearest(P, sunDir, planetCenter, bottomRadius) >= 0.f;
}

// Transmittance from P to the top of the atmosphere along sunDir, computed
// with a short nested ray march (extinction only, no scattering) -- the
// paper precomputes this into a 2D LUT indexed by height and sun zenith
// angle instead of evaluating it on the fly.
inline vec3 sunTransmittance(const vec3 &P, const vec3 &sunDir) {
  const vec3 planetCenter(0.f);
  float tTop = raySphereIntersectNearest(P, sunDir, planetCenter, topRadius);
  if (tTop < 0.f) {
    return vec3(0.f);
  }

  const int sunSampleCount = 8;
  float dt = tTop / sunSampleCount;
  vec3 opticalDepth(0.f);
  for (int i = 0; i < sunSampleCount; ++i) {
    vec3 samplePos = P + sunDir * (dt * (i + 0.5f));
    float height = length(samplePos - planetCenter) - bottomRadius;
    opticalDepth += sampleMedium(height).extinction * dt;
  }
  return exp(-opticalDepth);
}

inline vec3 sampleSunDisk(const vec3 &rayOrigin, const vec3 &rayDir, const vec3 &sunDir) {
  if (dot(rayDir, sunDir) <= cosf(sunAngularRadius)) {
    return vec3(0.f);
  }
  // Don't draw the disk if the sun is below the horizon from here.
  if (inEarthShadow(rayOrigin, sunDir)) {
    return vec3(0.f);
  }
  return sunIlluminance * sunDiskLuminance;
}

// The single-scattering raymarch itself (Hillaire EGSR2020, Section 4),
// plus the ambient multi-scattering fudge term and exposure/Reinhard
// tonemap described in the file comment above. Pure function of its
// arguments and compile-time constants -- safe to call from any thread.
inline vec3 computeRadiance(const vec3 &rayDir, const vec3 &sunDir,
                            float exposure, float multiScatteringFactor) {
  const vec3 planetCenter(0.f);
  const vec3 rayOrigin(0.f, bottomRadius + eyeHeightKm, 0.f);

  float tBottom = raySphereIntersectNearest(rayOrigin, rayDir, planetCenter, bottomRadius);
  float tTop = raySphereIntersectNearest(rayOrigin, rayDir, planetCenter, topRadius);

  if (tTop < 0.f) {
    // Ray misses the atmosphere entirely (shouldn't happen from inside it,
    // but guard against grazing/edge-case directions).
    return vec3(0.f);
  }
  float tMax = tBottom >= 0.f ? tBottom : tTop;

  float cosTheta = dot(sunDir, rayDir);
  float phaseRayleigh = rayleighPhase(cosTheta);
  // Negated to match the physical scattering angle (angle between the
  // direction light arrives FROM the sun and the direction we're looking
  // along), not the angle between the two "toward" directions.
  float phaseMie = miePhase(mieG, -cosTheta);

  const int sampleCount = 24;
  vec3 L(0.f);
  vec3 throughput(1.f);
  float t = 0.f;
  for (int i = 0; i < sampleCount; ++i) {
    float newT = tMax * (i + 0.5f) / sampleCount;
    float dt = newT - t;
    t = newT;

    vec3 P = rayOrigin + rayDir * t;
    float height = length(P - planetCenter) - bottomRadius;

    Medium medium = sampleMedium(height);
    vec3 sampleOpticalDepth = medium.extinction * dt;
    vec3 sampleTransmittance = exp(-sampleOpticalDepth);

    vec3 phaseTimesScattering =
        medium.scatteringMie * phaseMie + medium.scatteringRayleigh * phaseRayleigh;

    vec3 transmittanceToSun = inEarthShadow(P, sunDir)
        ? vec3(0.f)
        : sunTransmittance(P, sunDir);

    // Simple stand-in for the paper's multi-scattering LUT: treat a
    // fraction of the locally single-scattered light as if it also arrives
    // here isotropically, instead of actually integrating it.
    vec3 totalScattering = medium.scatteringRayleigh + medium.scatteringMie;
    vec3 multiScatter = totalScattering * transmittanceToSun * multiScatteringFactor;

    vec3 S = sunIlluminance * (transmittanceToSun * phaseTimesScattering + multiScatter);

    // Analytic per-segment integration of S*exp(-extinction*t') over the
    // step, rather than a plain Riemann sum -- avoids banding at low sample
    // counts (see the "slide 28" trick this is based on:
    // http://www.frostbite.com/2015/08/physically-based-unified-volumetric-rendering-in-frostbite/).
    vec3 safeExtinction = max(medium.extinction, vec3(1e-8f));
    vec3 Sint = (S - S * sampleTransmittance) / safeExtinction;
    L += throughput * Sint;
    throughput *= sampleTransmittance;
  }

  vec3 sunDisk = sampleSunDisk(rayOrigin, rayDir, sunDir) * throughput;

  // Simple exposure + Reinhard tonemap; nothing here is meant to be
  // radiometrically exact, just enough to keep the physically-driven color
  // relationships (blue zenith, reddening horizon at sunset, etc.) in a
  // displayable range.
  vec3 raw = (L + sunDisk) * exposure;
  return raw / (raw + vec3(1.f));
}

} // namespace atmosphere

// Maps a (not necessarily normalized) world-space direction to the uv
// coordinates of the equirectangular lat/long layout used by both
// bakeSkyTexture below and the shaders that sample it.
static vec2 directionToEquirectUv(const vec3 &dir) {
  vec3 d = normalize(dir);
  float elevation = asinf(glm::clamp(d.y, -1.f, 1.f));
  float azimuth = atan2f(d.z, d.x);
  return vec2(azimuth / glm::two_pi<float>() + 0.5f, 0.5f - elevation / glm::pi<float>());
}

// Renders the atmosphere per pixel, every frame -- kept alongside the baked
// version below so this demo can compare the two directly.
class AtmosphereFragmentShader : public FragmentShader {
public:
  Fragment shadeSingle(const ShadingGeometry &in) override {
    vec3 rayDir = normalize(vec3(in.varying[0]));
    vec3 radiance = atmosphere::computeRadiance(rayDir, normalize(sunDirection), exposure, multiScatteringFactor);

    Fragment out;
    out.color = vec4(radiance, 1.f);
    out.discard = false;
    return out;
  }

  // Direction from the scene toward the sun (does not need to be normalized).
  vec3 sunDirection = vec3(0.f, 1.f, 0.f);

  // Overall brightness before tonemapping.
  float exposure = 12.f;

  // Fudge factor for the approximate multi-scattering term -- 0 disables it
  // entirely (pure single scattering).
  float multiScatteringFactor = 0.5f;
};

// Looks up the sky color from a baked equirectangular texture instead of
// computing it. The texture is produced by Demo11::bakeSkyTexture below and
// handed to us once per frame; see that function for the lat/long layout.
class LatLongSkyboxFragmentShader : public FragmentShader {
public:
  Fragment shadeSingle(const ShadingGeometry &in) override {
    vec2 uv = directionToEquirectUv(vec3(in.varying[0]));

    Fragment out;
    out.color = texture ? texture->getTexel(uv) : vec4(0.f, 0.f, 0.f, 1.f);
    out.discard = false;
    return out;
  }

  // Set once per frame by Demo11 before drawing. Safe to read for the
  // whole frame: Texture is immutable after construction, and holding our
  // own shared_ptr keeps that instance alive even if the bake thread
  // publishes a newer texture mid-frame.
  std::shared_ptr<Texture> texture;
};

// Shades a surface as a perfect mirror reflecting the sky: reflects the view
// direction off the (smooth, per-vertex-interpolated) surface normal and
// looks the result up in the same sky representation the skybox itself is
// currently using -- the baked lat/long texture in BakedAtmosphere mode, or
// a direct raymarch in LiveAtmosphere mode -- so toggling 'n' compares the
// two consistently for both the sky and its reflection.
class ReflectiveSkyboxFragmentShader : public FragmentShader {
public:
  Fragment shadeSingle(const ShadingGeometry &in) override {
    vec3 viewDir = normalize(in.position - eyePosition);
    vec3 normal = normalize(in.normal);
    vec3 reflectDir = glm::reflect(viewDir, normal);

    vec3 radiance;
    if (useLiveAtmosphere) {
      radiance = atmosphere::computeRadiance(reflectDir, normalize(sunDirection), exposure, multiScatteringFactor);
    } else {
      radiance = texture ? vec3(texture->getTexel(directionToEquirectUv(reflectDir))) : vec3(0.f);
    }

    Fragment out;
    out.color = vec4(radiance, 1.f);
    out.discard = false;
    return out;
  }

  // All set once per frame by Demo11 before drawing; see LatLongSkyboxFragmentShader
  // above for why holding our own shared_ptr to `texture` is safe.
  vec3 eyePosition = vec3(0.f);
  vec3 sunDirection = vec3(0.f, 1.f, 0.f);
  float exposure = 12.f;
  float multiScatteringFactor = 0.5f;
  bool useLiveAtmosphere = false;
  std::shared_ptr<Texture> texture;
};

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
    dynamic_cast<OrbitCamera*>(camera.get())->setTarget(bunny.getCenter());

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

    std::vector<vec4> data(kSkyLatLongWidth * kSkyLatLongHeight);
    for (unsigned int j = 0; j < kSkyLatLongHeight; ++j) {
      float v = (j + 0.5f) / kSkyLatLongHeight;
      float elevation = (0.5f - v) * glm::pi<float>();
      for (unsigned int i = 0; i < kSkyLatLongWidth; ++i) {
        float u = (i + 0.5f) / kSkyLatLongWidth;
        float azimuth = (u - 0.5f) * glm::two_pi<float>();
        vec3 dir(cosf(elevation) * cosf(azimuth), sinf(elevation), cosf(elevation) * sinf(azimuth));
        vec3 radiance = atmosphere::computeRadiance(dir, sunDir, atmosphereExposure, atmosphereMultiScatter);
        data[i + j * kSkyLatLongWidth] = vec4(radiance, 1.f);
      }
    }

    std::shared_ptr<Texture> newTexture(Texture::fromVec4s(kSkyLatLongWidth, kSkyLatLongHeight, data));

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
