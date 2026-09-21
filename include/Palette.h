#ifndef GFX1993_PALETTE_INCLUDED
#define GFX1993_PALETTE_INCLUDED

#include <array>
#include <cstdint>

#include <glm/glm.hpp>

namespace gfx1993 {

// A fixed 256-color palette plus a fast nearest-color quantizer, for
// retro/VGA-style palettized output as a post-process over an otherwise
// full-float-RGB render -- render and shade in float RGB as normal, and
// only quantize once, at the very end (e.g. when converting the
// framebuffer to a displayable format), rather than intercepting every
// Framebuffer::plot() call. That keeps alpha blending working in full
// precision instead of compounding quantization error on every blend.
//
// Nearest-color lookup is a precomputed 3D grid over RGB space (a
// kLutResolution^3 lookup table), not a linear scan of all 256 colors per
// pixel: each cell stores the palette index nearest to that cell's center,
// computed once when the palette is built. This is functionally a
// discretized Voronoi diagram over color space -- same effect as an
// explicit Voronoi/kd-tree structure, without needing one.
class Palette {
public:
  static constexpr int kColorCount = 256;
  // Per-channel resolution of the nearest-color lookup table, i.e. the
  // table has kLutResolution^3 cells.
  static constexpr int kLutResolution = 32;
  static constexpr int kLutSize = kLutResolution * kLutResolution * kLutResolution;

  // Builds the classic default VGA/Mode-13h-style 256 color palette: the
  // 16 standard EGA colors, a 6x6x6 RGB color cube (the same level steps
  // as the "web safe" palette), and a 24-step grayscale ramp filling the
  // rest.
  static Palette makeStandardVga();

  // Nearest palette color to a linear RGB color with components roughly in
  // [0,1] (out-of-range components are clamped). O(1): a LUT cell lookup,
  // no search.
  inline glm::vec3 quantize(const glm::vec3 &color) const {
    return colors[nearestIndex(color)];
  }

  // Same lookup, returning the palette index rather than its color.
  inline uint8_t nearestIndex(const glm::vec3 &color) const {
    return lut[lutCellIndex(color)];
  }

  inline const std::array<glm::vec3, kColorCount> &getColors() const { return colors; }

private:
  Palette() = default;

  // Fills `lut` by, for every cell, linearly scanning `colors` for the
  // nearest entry to that cell's center color. O(kLutResolution^3 *
  // kColorCount); called once when the palette is built, never per-frame.
  void buildLut();

  inline unsigned int lutCellIndex(const glm::vec3 &color) const {
    glm::ivec3 cell = glm::clamp(glm::ivec3(color * static_cast<float>(kLutResolution)),
                                  0, kLutResolution - 1);
    return cell.x + kLutResolution * (cell.y + kLutResolution * cell.z);
  }

  std::array<glm::vec3, kColorCount> colors{};  // linear RGB in [0,1]
  std::array<uint8_t, kLutSize> lut{};          // indices into colors
};

}  // namespace gfx1993

#endif  // GFX1993_PALETTE_INCLUDED
