#include "Palette.h"

#include <cassert>
#include <limits>

namespace gfx1993 {

namespace {

inline glm::vec3 rgb8(int r, int g, int b) {
  return glm::vec3(r, g, b) / 255.f;
}

}  // namespace

Palette Palette::makeStandardVga() {
  Palette p;
  int idx = 0;

  // The 16 standard CGA/EGA/VGA text-mode colors.
  static constexpr int kEga[16][3] = {
      {0, 0, 0},       {0, 0, 170},     {0, 170, 0},     {0, 170, 170},
      {170, 0, 0},     {170, 0, 170},   {170, 85, 0},    {170, 170, 170},
      {85, 85, 85},    {85, 85, 255},   {85, 255, 85},   {85, 255, 255},
      {255, 85, 85},   {255, 85, 255},  {255, 255, 85},  {255, 255, 255},
  };
  for (const auto &c : kEga) {
    p.colors[idx++] = rgb8(c[0], c[1], c[2]);
  }

  // A 6x6x6 RGB color cube -- the same level steps (0, 51, 102, 153, 204,
  // 255) as the "web safe" 216-color palette.
  static constexpr int kLevels[6] = {0, 51, 102, 153, 204, 255};
  for (int r = 0; r < 6; ++r) {
    for (int g = 0; g < 6; ++g) {
      for (int b = 0; b < 6; ++b) {
        p.colors[idx++] = rgb8(kLevels[r], kLevels[g], kLevels[b]);
      }
    }
  }

  // Fill the remaining slots (256 - 16 - 216 = 24) with a grayscale ramp
  // strictly between black and white (both already present above, as EGA
  // black/white and the color cube's (0,0,0)/(255,255,255) corners).
  const int grayCount = kColorCount - idx;
  for (int i = 1; i <= grayCount; ++i) {
    int level = i * 255 / (grayCount + 1);
    p.colors[idx++] = rgb8(level, level, level);
  }
  assert(idx == kColorCount);

  p.buildLut();
  return p;
}

void Palette::buildLut() {
  for (int z = 0; z < kLutResolution; ++z) {
    for (int y = 0; y < kLutResolution; ++y) {
      for (int x = 0; x < kLutResolution; ++x) {
        glm::vec3 cellCenter = (glm::vec3(x, y, z) + 0.5f) / static_cast<float>(kLutResolution);

        uint8_t nearest = 0;
        float nearestDistSq = std::numeric_limits<float>::max();
        for (size_t i = 0; i < colors.size(); ++i) {
          glm::vec3 diff = colors[i] - cellCenter;
          float distSq = glm::dot(diff, diff);
          if (distSq < nearestDistSq) {
            nearestDistSq = distSq;
            nearest = static_cast<uint8_t>(i);
          }
        }

        lut[x + kLutResolution * (y + kLutResolution * z)] = nearest;
      }
    }
  }
}

}  // namespace gfx1993
