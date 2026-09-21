#include <GUnit.h>

#include <glm/ext.hpp>

#include "Palette.h"

namespace gfx1993 {

using glm::vec3;

namespace {

inline float distance2(const vec3& a, const vec3& b) {
  vec3 d = a - b;
  return glm::dot(d, d);
}

}  // namespace

GTEST("Palette Test") {
  SHOULD("Have exactly 256 colors, all within [0,1]") {
    Palette palette = Palette::makeStandardVga();

    EXPECT(palette.getColors().size() == Palette::kColorCount);
    for (const vec3& c : palette.getColors()) {
      EXPECT(c.r >= 0.f && c.r <= 1.f);
      EXPECT(c.g >= 0.f && c.g <= 1.f);
      EXPECT(c.b >= 0.f && c.b <= 1.f);
    }
  }

  SHOULD("Quantize black and white to themselves") {
    Palette palette = Palette::makeStandardVga();

    EXPECT(distance2(palette.quantize(vec3(0.f)), vec3(0.f)) < 1e-6f);
    EXPECT(distance2(palette.quantize(vec3(1.f)), vec3(1.f)) < 1e-6f);
  }

  SHOULD("Quantize every palette color close to itself") {
    Palette palette = Palette::makeStandardVga();

    // A LUT cell tie could in principle pick a different, equally-near
    // entry, so allow a small tolerance rather than requiring the exact
    // same index -- but the result should always be very close to the
    // original color, never some unrelated palette entry.
    for (const vec3& c : palette.getColors()) {
      EXPECT(distance2(palette.quantize(c), c) < 0.01f);
    }
  }

  SHOULD("Return a color consistent with nearestIndex") {
    Palette palette = Palette::makeStandardVga();

    vec3 sample(0.42f, 0.17f, 0.83f);
    uint8_t index = palette.nearestIndex(sample);

    EXPECT(index < Palette::kColorCount);
    EXPECT(palette.getColors()[index] == palette.quantize(sample));
  }

  SHOULD("Be idempotent -- quantizing a quantized color returns the same color") {
    Palette palette = Palette::makeStandardVga();

    vec3 sample(0.6f, 0.31f, 0.9f);
    vec3 once = palette.quantize(sample);
    vec3 twice = palette.quantize(once);

    EXPECT(once == twice);
  }

  SHOULD("Clamp out-of-range components instead of crashing") {
    Palette palette = Palette::makeStandardVga();

    vec3 result = palette.quantize(vec3(-1.f, 2.f, 0.5f));
    EXPECT(result.r >= 0.f && result.r <= 1.f);
    EXPECT(result.g >= 0.f && result.g <= 1.f);
    EXPECT(result.b >= 0.f && result.b <= 1.f);
  }

  SHOULD("Dither to an actual palette entry at every pixel") {
    Palette palette = Palette::makeStandardVga();
    const auto& colors = palette.getColors();

    vec3 sample(0.6f, 0.8f, 0.9f);
    for (unsigned int y = 0; y < 8; ++y) {
      for (unsigned int x = 0; x < 8; ++x) {
        vec3 out = palette.quantizeDithered(sample, x, y);
        bool isRealEntry = false;
        for (const vec3& c : colors) {
          if (c == out) {
            isRealEntry = true;
            break;
          }
        }
        EXPECT(isRealEntry);
      }
    }
  }

  SHOULD("Spread a color that falls between two palette levels across a pattern") {
    Palette palette = Palette::makeStandardVga();

    // Deliberately not a palette entry itself -- roughly midway between
    // adjacent color-cube levels on more than one channel.
    vec3 sample(0.6f, 0.8f, 0.9f);

    int distinctCount = 0;
    vec3 seen[64];
    for (unsigned int y = 0; y < 8; ++y) {
      for (unsigned int x = 0; x < 8; ++x) {
        vec3 out = palette.quantizeDithered(sample, x, y);
        bool isNew = true;
        for (int i = 0; i < distinctCount; ++i) {
          if (seen[i] == out) {
            isNew = false;
            break;
          }
        }
        if (isNew) {
          seen[distinctCount++] = out;
        }
      }
    }

    EXPECT(distinctCount > 1);
  }

  SHOULD("Average closer to the true color than a single flat quantize") {
    Palette palette = Palette::makeStandardVga();

    vec3 sample(0.6f, 0.8f, 0.9f);
    vec3 flat = palette.quantize(sample);

    vec3 sum(0.f);
    for (unsigned int y = 0; y < 8; ++y) {
      for (unsigned int x = 0; x < 8; ++x) {
        sum += palette.quantizeDithered(sample, x, y);
      }
    }
    vec3 average = sum / 64.f;

    EXPECT(distance2(average, sample) < distance2(flat, sample));
  }
}

}  // namespace gfx1993
