#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/transform.hpp>

#include "DemoApp.h"
#include "DemoUtils.h"
#include "Geometry.h"
#include "Pipeline.h"
#include "Profiler.h"
#include "Shader.h"
#include "Camera.h"

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
// instead ray-marches the single-scattering integral directly per pixel
// (the paper's core physical model, Section 4) -- it's slower, but the
// screen is small and we already have a thread pool parallelizing the fill.
// Multiple scattering (Section 5.2's precomputed LUT, built from an
// iterative dual-scattering approximation) is out of scope for a per-pixel
// shader with no prepass, so it's replaced with a simple ambient fudge term
// (see multiScatteringFactor below) rather than reproduced faithfully.
class AtmosphereFragmentShader : public FragmentShader {
public:
  Fragment shadeSingle(const ShadingGeometry &in) override {
    vec3 rayDir = normalize(vec3(in.varying[0]));
    vec3 sunDir = normalize(sunDirection);

    const vec3 planetCenter(0.f);
    const vec3 rayOrigin(0.f, bottomRadius + eyeHeightKm, 0.f);

    float tBottom = raySphereIntersectNearest(rayOrigin, rayDir, planetCenter, bottomRadius);
    float tTop = raySphereIntersectNearest(rayOrigin, rayDir, planetCenter, topRadius);

    Fragment out;
    if (tTop < 0.f) {
      // Ray misses the atmosphere entirely (shouldn't happen from inside
      // it, but guard against grazing/edge-case directions).
      out.color = vec4(0.f, 0.f, 0.f, 1.f);
      return out;
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
      // fraction of the locally single-scattered light as if it also
      // arrives here isotropically, instead of actually integrating it.
      vec3 totalScattering = medium.scatteringRayleigh + medium.scatteringMie;
      vec3 multiScatter = totalScattering * transmittanceToSun * multiScatteringFactor;

      vec3 S = sunIlluminance * (transmittanceToSun * phaseTimesScattering + multiScatter);

      // Analytic per-segment integration of S*exp(-extinction*t') over the
      // step, rather than a plain Riemann sum -- avoids banding at low
      // sample counts (see the "slide 28" trick this is based on:
      // http://www.frostbite.com/2015/08/physically-based-unified-volumetric-rendering-in-frostbite/).
      vec3 safeExtinction = max(medium.extinction, vec3(1e-8f));
      vec3 Sint = (S - S * sampleTransmittance) / safeExtinction;
      L += throughput * Sint;
      throughput *= sampleTransmittance;
    }

    vec3 sunDisk = sampleSunDisk(rayOrigin, rayDir, sunDir) * throughput;

    // Simple exposure + Reinhard tonemap; nothing here is meant to be
    // radiometrically exact, just enough to keep the physically-driven
    // color relationships (blue zenith, reddening horizon at sunset, etc.)
    // in a displayable range.
    vec3 raw = (L + sunDisk) * exposure;
    vec3 tonemapped = raw / (raw + vec3(1.f));

    out.color = vec4(tonemapped, 1.f);
    out.discard = false;
    return out;
  }

  // Direction from the scene toward the sun (does not need to be normalized).
  vec3 sunDirection = vec3(0.f, 1.f, 0.f);

  // Overall brightness before tonemapping.
  float exposure = 12.f;

  // Fudge factor for the approximate multi-scattering term (see class
  // comment above) -- 0 disables it entirely (pure single scattering).
  float multiScatteringFactor = 0.5f;

private:
  struct Medium {
    vec3 scatteringRayleigh;
    vec3 scatteringMie;
    vec3 extinction;
  };

  // All distances in kilometers, matching the paper's own units and its
  // reference implementation's Earth preset
  // (https://github.com/sebh/UnrealEngineSkyAtmosphere).
  static constexpr float bottomRadius = 6360.f;
  static constexpr float topRadius = 6460.f;
  static constexpr float rayleighScaleHeight = 8.f;
  static constexpr float mieScaleHeight = 1.2f;
  static constexpr float mieG = 0.8f;
  // A fixed, token "eye level" viewer height above the ground. Since this
  // shader only ever sees a view direction (it's a skybox, rendered
  // independent of the scene's own tiny local camera position/units), the
  // sky's appearance only depends on this height and the view/sun angles.
  static constexpr float eyeHeightKm = 0.2f;

  static constexpr vec3 rayleighScattering = vec3(0.005802f, 0.013558f, 0.033100f); // 1/km
  static constexpr vec3 mieScattering = vec3(0.003996f);                            // 1/km
  static constexpr vec3 mieExtinction = vec3(0.004440f);                            // 1/km
  static constexpr vec3 ozoneAbsorption = vec3(0.000650f, 0.001881f, 0.000085f);    // 1/km

  static constexpr vec3 sunIlluminance = vec3(1.f);
  static constexpr float sunAngularRadius = 0.004675f; // radians
  static constexpr float sunDiskLuminance = 4000.f;

  static float raySphereIntersectNearest(const vec3 &rayOrigin, const vec3 &rayDir,
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

  static float rayleighPhase(float cosTheta) {
    const float k = 3.f / (16.f * glm::pi<float>());
    return k * (1.f + cosTheta * cosTheta);
  }

  // Cornette-Shanks approximation of the Mie phase function.
  static float miePhase(float g, float cosTheta) {
    const float k = 3.f / (8.f * glm::pi<float>()) * (1.f - g * g) / (2.f + g * g);
    return k * (1.f + cosTheta * cosTheta) /
          powf(1.f + g * g - 2.f * g * cosTheta, 1.5f);
  }

  // Rayleigh/Mie density falls off exponentially with height; ozone forms a
  // tent-shaped layer peaking around 25km (both pieces evaluate to 1 there).
  Medium sampleMedium(float height) const {
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

  bool inEarthShadow(const vec3 &P, const vec3 &sunDir) const {
    const vec3 planetCenter(0.f);
    return raySphereIntersectNearest(P, sunDir, planetCenter, bottomRadius) >= 0.f;
  }

  // Transmittance from P to the top of the atmosphere along sunDir,
  // computed with a short nested ray march (extinction only, no
  // scattering) -- the paper precomputes this into a 2D LUT indexed by
  // height and sun zenith angle instead of evaluating it on the fly.
  vec3 sunTransmittance(const vec3 &P, const vec3 &sunDir) const {
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

  vec3 sampleSunDisk(const vec3 &rayOrigin, const vec3 &rayDir, const vec3 &sunDir) const {
    if (dot(rayDir, sunDir) <= cosf(sunAngularRadius)) {
      return vec3(0.f);
    }
    // Don't draw the disk if the sun is below the horizon from here.
    if (inEarthShadow(rayOrigin, sunDir)) {
      return vec3(0.f);
    }
    return sunIlluminance * sunDiskLuminance;
  }
};

class Demo10 : public DemoApp {
public:
  Demo10() : DemoApp("Demo 10 - Skybox"), skybox(vec4(1.f)) {}

protected:
  void init() override {
    fixedFunctionTransform = std::make_shared<DefaultVertexTransform>();
    gridShader = std::make_shared<InputColorShader>();

    skyboxVertShader = std::make_shared<SkyboxVertexShader>();
    skyboxFragShader = std::make_shared<SkyboxFragmentShader>(
      vec3(0.7f, 0.7f, 1.f),
      vec3(0.8f, 0.8f, 0.9f),
      vec3(0.3f, 0.3f, 0.4f));
    atmosphereFragShader = std::make_shared<AtmosphereFragmentShader>();

    grid = std::make_unique<GridGeometry>();

    teapot.makeIndicesForPointCloud();
    colorShader = std::make_unique<SingleColorShader>(vec4(1,0,1,1));

    std::cout << "Press 'n' to toggle the physically based atmosphere shader, "
                 "'p' to pause/resume the day/night cycle, "
                 "'[' / ']' to scrub time of day while paused."
              << std::endl;
  }

  void updateFrame(float dt) override {
    DemoApp::updateFrame(dt);

    if (!daylightPaused) {
      dayTime += dt / dayLengthSeconds;
      dayTime -= floorf(dayTime);
    }

    // dayTime: 0 = midnight, 0.25 = sunrise, 0.5 = noon, 0.75 = sunset.
    float elevation = sinf(glm::two_pi<float>() * (dayTime - 0.25f)) * glm::half_pi<float>();
    vec3 sunDir(sinf(sunAzimuth) * cosf(elevation), sinf(elevation), cosf(sunAzimuth) * cosf(elevation));
    atmosphereFragShader->sunDirection = sunDir;
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
    renderConfig.fragmentShader = useAtmosphereShader
        ? std::static_pointer_cast<FragmentShader>(atmosphereFragShader)
        : std::static_pointer_cast<FragmentShader>(skyboxFragShader);
    rasterizer->drawTriangles(renderConfig, skybox.getVertices(), skybox.getIndices());

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

    renderConfig.fragmentShader = colorShader;
    rasterizer->drawPoints(renderConfig, teapot.getVertices(), teapot.getIndices());
  }

  void handleKeyboard(unsigned char key, const glm::ivec2& mouse) override {
    if (key == '=') {
      renderConfig.pointSize += 2;
    }
    if (key == '-') {
      renderConfig.pointSize -= 2;
    }
    renderConfig.pointSize = glm::clamp(renderConfig.pointSize, 1u, 11u);

    if (key == 'n') {
      useAtmosphereShader = !useAtmosphereShader;
      std::cout << "Sky shader: "
                << (useAtmosphereShader ? "physically based atmosphere" : "gradient skybox")
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
  }


private:
  std::unique_ptr<GridGeometry> grid;

  std::shared_ptr<DefaultVertexTransform> fixedFunctionTransform;
  std::shared_ptr<FragmentShader> gridShader;
  std::shared_ptr<SingleColorShader> colorShader;

  std::shared_ptr<SkyboxVertexShader>   skyboxVertShader;
  std::shared_ptr<SkyboxFragmentShader> skyboxFragShader;
  std::shared_ptr<AtmosphereFragmentShader> atmosphereFragShader;
  bool useAtmosphereShader = true;
  Quad skybox;

  // Time of day, in [0,1); see updateFrame().
  float dayTime = 0.4f;
  float dayLengthSeconds = 60.f;
  bool daylightPaused = false;
  // Fixed compass direction the sun arcs through, in radians.
  float sunAzimuth = 0.6f;

  Teapot teapot;
};

int main(int argc, char **argv) {
  Demo10 demo;
  demo.run(argc, argv);

  return 0;
}
