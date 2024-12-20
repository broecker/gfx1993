#ifndef GFX1993_RENDERDEBUGINFO_H
#define GFX1993_RENDERDEBUGINFO_H

#include <cstdint>

namespace gfx1993 {
namespace render {

struct PrimitiveDebugInfo {
  uint32_t drawn;
  uint32_t fragmentsDrawn;
  uint32_t fragmentsDiscarded;
};

// Contains rasterization debug information.
struct DebugInfo {
  // Rasterization information.
  PrimitiveDebugInfo points;
  PrimitiveDebugInfo lines;
  PrimitiveDebugInfo triangles;

  void reset();
  void print() const;
};

}  // namespace rendder
}  // namespace gfx1993

#endif // GFX1993_RENDERDEBUGINFO_H
