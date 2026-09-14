#include "Atmosphere.h"

#include <cmath>

#include <glm/ext.hpp>
#include <glm/gtc/constants.hpp>

using namespace glm;

namespace gfx1993 {
namespace atmosphere {

namespace {

// Returns both roots of the ray/sphere intersection (t0 <= t1), regardless
// of sign -- the caller decides how to interpret a ray origin that's inside
// the sphere (one or both roots negative) or pointed away from it (both
// roots negative). Returns false if the ray misses the sphere entirely.
inline bool raySphereIntersect(const vec3 &rayOrigin, const vec3 &rayDir,
                               const vec3 &sphereCenter, float sphereRadius,
                               float &t0, float &t1) {
  vec3 oc = rayOrigin - sphereCenter;
  float b = dot(oc, rayDir);
  float c = dot(oc, oc) - sphereRadius * sphereRadius;
  float discriminant = b * b - c;
  if (discriminant < 0.f) {
    return false;
  }
  float sqrtD = sqrtf(discriminant);
  t0 = -b - sqrtD;
  t1 = -b + sqrtD;
  return true;
}

inline float raySphereIntersectNearest(const vec3 &rayOrigin, const vec3 &rayDir,
                                       const vec3 &sphereCenter, float sphereRadius) {
  float t0, t1;
  if (!raySphereIntersect(rayOrigin, rayDir, sphereCenter, sphereRadius, t0, t1)) {
    return -1.f;
  }
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

} // namespace

RadianceSample computeRadiance(const vec3 &rayOrigin, const vec3 &rayDir,
                               const vec3 &sunDir, float exposure,
                               float multiScatteringFactor) {
  const vec3 planetCenter(0.f);

  // Range of the shell we actually need to march: from where the ray enters
  // the top of the atmosphere (or 0, if rayOrigin is already inside it) to
  // wherever it leaves again -- the ground, or the far side of the shell.
  // This is what lets rayOrigin be anywhere, not just already inside the
  // atmosphere: a camera far out in orbit has a positive tTopNear (the shell
  // is somewhere ahead of it), so the march starts there instead of wasting
  // samples on the vacuum in between.
  float tTopNear, tTopFar;
  if (!raySphereIntersect(rayOrigin, rayDir, planetCenter, topRadius, tTopNear, tTopFar) ||
      tTopFar < 0.f) {
    // Ray never touches the atmosphere: looking into deep space, or away
    // from the planet entirely.
    return RadianceSample{vec3(0.f), 1.f};
  }
  float tEntry = glm::max(tTopNear, 0.f);

  float tBottomNear = raySphereIntersectNearest(rayOrigin, rayDir, planetCenter, bottomRadius);
  float tExit = tBottomNear >= 0.f ? tBottomNear : tTopFar;

  if (tExit <= tEntry) {
    // Grazing/degenerate case: no usable segment inside the shell.
    return RadianceSample{vec3(0.f), 1.f};
  }

  float cosTheta = dot(sunDir, rayDir);
  float phaseRayleigh = rayleighPhase(cosTheta);
  // Negated to match the physical scattering angle (angle between the
  // direction light arrives FROM the sun and the direction we're looking
  // along), not the angle between the two "toward" directions.
  float phaseMie = miePhase(mieG, -cosTheta);

  const int sampleCount = 24;
  vec3 L(0.f);
  vec3 throughput(1.f);
  float t = tEntry;
  for (int i = 0; i < sampleCount; ++i) {
    float newT = tEntry + (tExit - tEntry) * (i + 0.5f) / sampleCount;
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
  vec3 radiance = raw / (raw + vec3(1.f));

  float transmittance = (throughput.r + throughput.g + throughput.b) / 3.f;
  return RadianceSample{radiance, transmittance};
}

} // namespace atmosphere

vec2 directionToEquirectUv(const vec3 &dir) {
  vec3 d = normalize(dir);
  float elevation = asinf(glm::clamp(d.y, -1.f, 1.f));
  float azimuth = atan2f(d.z, d.x);
  return vec2(azimuth / glm::two_pi<float>() + 0.5f, 0.5f - elevation / glm::pi<float>());
}

std::unique_ptr<Texture> bakeSkyLatLongTexture(const vec3 &rayOrigin, const vec3 &sunDir,
                                               float exposure, float multiScatteringFactor,
                                               unsigned int width, unsigned int height) {
  std::vector<vec4> data(width * height);
  for (unsigned int j = 0; j < height; ++j) {
    float v = (j + 0.5f) / height;
    float elevation = (0.5f - v) * glm::pi<float>();
    for (unsigned int i = 0; i < width; ++i) {
      float u = (i + 0.5f) / width;
      float azimuth = (u - 0.5f) * glm::two_pi<float>();
      vec3 dir(cosf(elevation) * cosf(azimuth), sinf(elevation), cosf(elevation) * sinf(azimuth));
      atmosphere::RadianceSample sample =
          atmosphere::computeRadiance(rayOrigin, dir, sunDir, exposure, multiScatteringFactor);
      data[i + j * width] = vec4(sample.radiance, 1.f);
    }
  }
  return Texture::fromVec4s(width, height, data);
}

Fragment AtmosphereFragmentShader::shadeSingle(const ShadingGeometry &in) {
  vec3 rayDir = normalize(vec3(in.varying[0]));

  atmosphere::RadianceSample sample = atmosphere::computeRadiance(
      atmosphere::groundLevelPosition(), rayDir, normalize(sunDirection), exposure,
      multiScatteringFactor);

  Fragment out;
  out.color = vec4(sample.radiance, 1.f);
  out.discard = false;
  return out;
}

Fragment LatLongSkyboxFragmentShader::shadeSingle(const ShadingGeometry &in) {
  vec2 uv = directionToEquirectUv(vec3(in.varying[0]));

  Fragment out;
  out.color = texture ? texture->getTexel(uv) : vec4(0.f, 0.f, 0.f, 1.f);
  out.discard = false;
  return out;
}

Fragment ReflectiveSkyboxFragmentShader::shadeSingle(const ShadingGeometry &in) {
  vec3 viewDir = normalize(in.position - eyePosition);
  vec3 normal = normalize(in.normal);
  vec3 reflectDir = glm::reflect(viewDir, normal);

  vec3 radiance;
  if (useLiveAtmosphere) {
    radiance = atmosphere::computeRadiance(atmosphere::groundLevelPosition(), reflectDir,
                                           normalize(sunDirection), exposure,
                                           multiScatteringFactor).radiance;
  } else {
    radiance = texture ? vec3(texture->getTexel(directionToEquirectUv(reflectDir))) : vec3(0.f);
  }

  Fragment out;
  out.color = vec4(radiance, 1.f);
  out.discard = false;
  return out;
}

Fragment PositionalAtmosphereFragmentShader::shadeSingle(const ShadingGeometry &in) {
  vec3 rayDir = normalize(vec3(in.varying[0]));
  atmosphere::RadianceSample sample = atmosphere::computeRadiance(
      cameraPosition, rayDir, normalize(sunDirection), exposure, multiScatteringFactor);

  Fragment out;
  out.color = vec4(sample.radiance, 1.f - sample.transmittance);
  out.discard = false;
  return out;
}

} // namespace gfx1993
