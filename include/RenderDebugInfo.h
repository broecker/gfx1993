#ifndef GFX1993_RENDERDEBUGINFO_H
#define GFX1993_RENDERDEBUGINFO_H

#include <cstdint>

namespace gfx1993 {
namespace util {

struct PrimitiveDebugInfo {
  uint32_t processed = 0;
  uint32_t drawn = 0;
  uint32_t backfaceCulled = 0;

  uint32_t fragmentsDrawn = 0;
  uint32_t fragmentsDiscarded = 0;
};

// Contains rasterization debug information.
struct DebugInfo {
  // Rasterization information.
  PrimitiveDebugInfo points;
  PrimitiveDebugInfo lines;
  PrimitiveDebugInfo triangles;
  PrimitiveDebugInfo screenFillingQuad;

  // We can also use this to fill in higher-level objects, such as bounding
  // boxes that should be culled /before/ we get to the rasterizer stage.
  PrimitiveDebugInfo aabbs;

  void reset();
  void print() const;
};

}  // namespace util
}  // namespace gfx1993

#endif // GFX1993_RENDERDEBUGINFO_H
