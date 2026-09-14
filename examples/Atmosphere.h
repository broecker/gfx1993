#ifndef GFX1993_EXAMPLES_ATMOSPHERE_H
#define GFX1993_EXAMPLES_ATMOSPHERE_H

#include <memory>

#include <glm/glm.hpp>

#include "Pipeline.h"
#include "Shader.h"
#include "Texture.h"

// Physically based atmospheric scattering, shared between demos that render
// a sky/atmosphere (11-dynamic-sky, 14-planet-atmosphere), based on:
//   Sebastien Hillaire, "A Scalable and Production Ready Sky and Atmosphere
//   Rendering Technique", EGSR 2020.
//   https://sebh.github.io/publications/egsr2020.pdf
//
// The paper's real-time trick is to precompute a transmittance LUT, a
// multi-scattering LUT, and a sky-view LUT on the GPU so the actual sky pixel
// shader is just a couple of texture fetches. gfx1993 is a CPU software
// rasterizer with no compute-shader/texture-write stage, so this instead
// ray-marches the single-scattering integral directly (the paper's core
// physical model, Section 4). Multiple scattering (Section 5.2's precomputed
// LUT, built from an iterative dual-scattering approximation) is out of scope
// for a per-pixel shader with no prepass, so it's replaced with a simple
// ambient fudge term (see multiScatteringFactor below).
namespace gfx1993 {
namespace atmosphere {

struct Medium {
  glm::vec3 scatteringRayleigh;
  glm::vec3 scatteringMie;
  glm::vec3 extinction;
};

// All distances in kilometers, matching the paper's own units and its
// reference implementation's Earth preset
// (https://github.com/sebh/UnrealEngineSkyAtmosphere).
constexpr float bottomRadius = 6360.f;
constexpr float topRadius = 6460.f;
constexpr float rayleighScaleHeight = 8.f;
constexpr float mieScaleHeight = 1.2f;
constexpr float mieG = 0.8f;

constexpr glm::vec3 rayleighScattering(0.005802f, 0.013558f, 0.033100f); // 1/km
constexpr glm::vec3 mieScattering(0.003996f);                            // 1/km
constexpr glm::vec3 mieExtinction(0.004440f);                            // 1/km
constexpr glm::vec3 ozoneAbsorption(0.000650f, 0.001881f, 0.000085f);    // 1/km

constexpr glm::vec3 sunIlluminance(1.f);
constexpr float sunAngularRadius = 0.004675f; // radians
constexpr float sunDiskLuminance = 4000.f;

// A fixed, token "eye level" viewer height above the ground, for callers
// that render the sky as a ground-level backdrop rather than a positioned
// volume (see groundLevelPosition below).
constexpr float defaultEyeHeightKm = 0.2f;

// A ground-level position, `eyeHeightKm` above bottomRadius, for demos that
// render the sky as an opaque backdrop instead of a positioned volume.
inline glm::vec3 groundLevelPosition(float eyeHeightKm = defaultEyeHeightKm) {
  return glm::vec3(0.f, bottomRadius + eyeHeightKm, 0.f);
}

struct RadianceSample {
  glm::vec3 radiance = glm::vec3(0.f);

  // Transmittance from rayOrigin to wherever the raymarch stopped: the
  // ground, or the far side of the atmosphere shell. 1 = the ray never
  // touched any medium (fully transparent); 0 = fully opaque. Callers that
  // treat the sky as an opaque backdrop (it's the only thing at that pixel)
  // can ignore this; callers that render the atmosphere as an actual
  // positioned object use `1 - transmittance` as alpha to composite over
  // whatever is behind it.
  float transmittance = 1.f;
};

// The single-scattering raymarch (Hillaire EGSR2020, Section 4), plus the
// ambient multi-scattering fudge term and exposure/Reinhard tonemap. Pure
// function of its arguments and the compile-time constants above -- safe to
// call from any thread (e.g. a background bake, see bakeSkyLatLongTexture).
//
// rayOrigin is in km, relative to the planet's center, and may be anywhere:
// on the ground, inside the shell, or far outside it in space. When the ray
// doesn't touch the atmosphere at all (e.g. looking from orbit out into deep
// space), this returns {vec3(0), 1.0} -- no radiance, fully transparent.
RadianceSample computeRadiance(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir,
                                const glm::vec3 &sunDir, float exposure,
                                float multiScatteringFactor);

} // namespace atmosphere

// Maps a (not necessarily normalized) world-space direction to the uv
// coordinates of the equirectangular lat/long layout used by both
// bakeSkyLatLongTexture and LatLongSkyboxFragmentShader.
glm::vec2 directionToEquirectUv(const glm::vec3 &dir);

// Bakes the sky into a width x height equirectangular texture as seen from
// rayOrigin (km, planet-relative). Pure/thread-safe like computeRadiance --
// safe to call on a background thread.
std::unique_ptr<Texture> bakeSkyLatLongTexture(const glm::vec3 &rayOrigin,
                                               const glm::vec3 &sunDir,
                                               float exposure,
                                               float multiScatteringFactor,
                                               unsigned int width,
                                               unsigned int height);

// Renders the atmosphere per pixel, live, from a fixed ground-level position
// -- opaque, meant to be the whole background. Pair with SkyboxVertexShader
// on a fullscreen quad, the same way a regular skybox is drawn.
class AtmosphereFragmentShader : public FragmentShader {
public:
  Fragment shadeSingle(const ShadingGeometry &in) override;

  // Direction from the scene toward the sun (does not need to be normalized).
  glm::vec3 sunDirection = glm::vec3(0.f, 1.f, 0.f);
  float exposure = 12.f;
  float multiScatteringFactor = 0.5f;
};

// Looks up the sky color from a baked equirectangular texture instead of
// computing it live. See bakeSkyLatLongTexture. Also opaque/ground-level.
class LatLongSkyboxFragmentShader : public FragmentShader {
public:
  Fragment shadeSingle(const ShadingGeometry &in) override;

  // Set once per frame by the demo. Safe to read for the whole frame:
  // Texture is immutable after construction, and holding our own shared_ptr
  // keeps this instance alive even if a bake thread publishes a newer one
  // mid-frame.
  std::shared_ptr<Texture> texture;
};

// Shades a surface as a mirror reflecting the sky -- either
// AtmosphereFragmentShader's live raymarch or LatLongSkyboxFragmentShader's
// baked texture, chosen by useLiveAtmosphere. Ground-level only (uses
// atmosphere::groundLevelPosition internally), matching the two shaders
// above.
class ReflectiveSkyboxFragmentShader : public FragmentShader {
public:
  Fragment shadeSingle(const ShadingGeometry &in) override;

  // All set once per frame by the demo before drawing; see
  // LatLongSkyboxFragmentShader above for why holding our own shared_ptr to
  // `texture` is safe.
  glm::vec3 eyePosition = glm::vec3(0.f);
  glm::vec3 sunDirection = glm::vec3(0.f, 1.f, 0.f);
  float exposure = 12.f;
  float multiScatteringFactor = 0.5f;
  bool useLiveAtmosphere = false;
  std::shared_ptr<Texture> texture;
};

// Renders the atmosphere as a real positioned volume around a planet, rather
// than a ground-level backdrop: the ray starts at the camera's actual
// position (km, planet-relative) instead of a fixed ground point, and alpha
// comes from the raymarch's transmittance so it composites over whatever's
// behind it -- stars, empty space -- instead of assuming it's the only thing
// at that pixel.
//
// Pair with SkyboxVertexShader on a fullscreen quad exactly like
// AtmosphereFragmentShader, but draw it *after* opaque scene geometry with
// RenderConfig::depthTest = true, depthWrite = false, alphaBlending = true --
// the quad's own clip depth is the far plane (see SkyboxVertexShader), so the
// depth test alone makes it disappear wherever nearer, opaque geometry (the
// planet) already occupies that pixel, giving a limb/rim-glow silhouette
// around the planet for free.
class PositionalAtmosphereFragmentShader : public FragmentShader {
public:
  Fragment shadeSingle(const ShadingGeometry &in) override;

  // Updated once per frame by the demo: the camera's position in km,
  // relative to the planet's center.
  glm::vec3 cameraPosition = glm::vec3(0.f);
  glm::vec3 sunDirection = glm::vec3(0.f, 1.f, 0.f);
  float exposure = 12.f;
  float multiScatteringFactor = 0.5f;
};

} // namespace gfx1993

#endif // GFX1993_EXAMPLES_ATMOSPHERE_H
