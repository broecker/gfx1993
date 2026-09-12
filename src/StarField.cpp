#include "StarField.h"
#include "Pipeline.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <limits>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace gfx1993 {

namespace {

// Coarse, piecewise-linear approximation of a star's visual color from its
// B-V color index -- a handful of reference stops (hot blue-white through
// cool red), linearly interpolated. This is a visual approximation, not a
// photometric blackbody model; the star's position is what needs to be
// astronomically correct, not its exact hue.
glm::vec3 colorIndexToRgb(float ci) {
  struct Stop { float ci; glm::vec3 color; };
  static const Stop stops[] = {
    {-0.4f, glm::vec3(0.61f, 0.70f, 1.00f)},  // O/B blue-white
    { 0.0f, glm::vec3(0.93f, 0.93f, 1.00f)},  // A white-blue
    { 0.4f, glm::vec3(1.00f, 0.95f, 0.85f)},  // F white-yellow
    { 0.65f, glm::vec3(1.00f, 0.89f, 0.71f)}, // G yellow (Sun-like)
    { 1.0f, glm::vec3(1.00f, 0.75f, 0.45f)},  // K orange
    { 1.6f, glm::vec3(1.00f, 0.55f, 0.35f)},  // M red-orange
    { 2.0f, glm::vec3(1.00f, 0.40f, 0.30f)},  // M red
  };
  constexpr int stopCount = sizeof(stops) / sizeof(stops[0]);

  if (ci <= stops[0].ci) return stops[0].color;
  if (ci >= stops[stopCount - 1].ci) return stops[stopCount - 1].color;

  for (int i = 0; i + 1 < stopCount; ++i) {
    if (ci >= stops[i].ci && ci <= stops[i + 1].ci) {
      float t = (ci - stops[i].ci) / (stops[i + 1].ci - stops[i].ci);
      return glm::mix(stops[i].color, stops[i + 1].color, t);
    }
  }
  return glm::vec3(1.f);
}

} // namespace

bool StarField::loadCatalog(const std::string &filename) {
  std::ifstream file(filename.c_str());
  if (!file.is_open()) {
    std::cerr << "[StarField] Unable to open file \"" << filename << "\"\n";
    return false;
  }

  struct RawStar { float ra, dec, mag, ci; };
  std::vector<RawStar> raw;

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') {
      continue;
    }

    // Columns are "name,ra_hours,dec_deg,mag,ci" -- the name field is
    // free text, so pull the four numeric fields from the end instead of
    // assuming the name itself contains no commas.
    float ra, dec, mag, ci;
    if (sscanf(line.c_str(), "%*[^,],%f,%f,%f,%f", &ra, &dec, &mag, &ci) != 4) {
      std::cerr << "[StarField] Malformed line: \"" << line << "\"\n";
      continue;
    }
    raw.push_back({ra, dec, mag, ci});
  }

  if (raw.empty()) {
    std::cerr << "[StarField] No stars loaded from \"" << filename << "\"\n";
    return false;
  }

  float minMag = std::numeric_limits<float>::max();
  float maxMag = std::numeric_limits<float>::lowest();
  for (const auto &s : raw) {
    minMag = std::min(minMag, s.mag);
    maxMag = std::max(maxMag, s.mag);
  }
  float magRange = std::max(maxMag - minMag, 1e-3f);

  catalog.clear();
  catalog.reserve(raw.size());
  for (const auto &s : raw) {
    CatalogStar star;
    star.raRadians = s.ra * (glm::pi<float>() / 12.f); // hours -> radians
    star.decRadians = glm::radians(s.dec);

    // Brightest star in the set -> 1.0, faintest -> still dimly visible.
    float t = (maxMag - s.mag) / magRange;
    float brightness = glm::mix(0.18f, 1.0f, glm::clamp(t, 0.f, 1.f));
    star.baseColor = colorIndexToRgb(s.ci) * brightness;

    catalog.push_back(star);
  }

  std::clog << "[StarField] Loaded " << catalog.size() << " stars from " << filename << std::endl;
  return true;
}

void StarField::updateVisibleStars(float siderealAngle, float observerLatitudeRadians,
                                   float distance, float brightnessScale) {
  vertices.clear();
  indices.clear();

  if (brightnessScale <= 0.f) {
    return;
  }

  const float sinLat = sinf(observerLatitudeRadians);
  const float cosLat = cosf(observerLatitudeRadians);

  vertices.reserve(catalog.size());
  for (const auto &star : catalog) {
    float hourAngle = siderealAngle - star.raRadians;

    float sinDec = sinf(star.decRadians);
    float cosDec = cosf(star.decRadians);

    float sinAlt = sinDec * sinLat + cosDec * cosLat * cosf(hourAngle);
    float alt = asinf(glm::clamp(sinAlt, -1.f, 1.f));
    if (alt <= 0.f) {
      continue; // below the horizon -- skip entirely.
    }

    float denom = cosf(alt) * cosLat;
    float cosAz = denom > 1e-6f
        ? glm::clamp((sinDec - sinf(alt) * sinLat) / denom, -1.f, 1.f)
        : 1.f;
    float az = acosf(cosAz);
    if (sinf(hourAngle) > 0.f) {
      az = glm::two_pi<float>() - az;
    }

    glm::vec3 dir(sinf(az) * cosf(alt), sinf(alt), cosf(az) * cosf(alt));
    glm::vec3 color = star.baseColor * brightnessScale;

    vertices.push_back(Vertex(glm::vec4(dir * distance, 1.f), glm::vec4(color, 1.f)));
  }

  makeIndicesForPointCloud();
}

} // namespace gfx1993
