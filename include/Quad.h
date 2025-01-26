#ifndef GFX1993_QUAD_H
#define GFX1993_QUAD_H

#include "Geometry.h"

#include <glm/glm.hpp>

namespace gfx1993 {
namespace geometry {

class Quad : public Geometry {
public:
  // Constructs a quad from 4 vertices. Assumes the following geometry:
  // a +---+ d
  //   | \ |
  // b +---+ c
  // The size will be [-1..1] along the XY axis and 0 on the z axis. It can be
  // used for screen-space rendering.
  // The quad is double-sided.
  explicit Quad(const glm::vec4 &color);

  // Single-size quad with the given size. The center point will be at (0,0,0).
  static Quad makeXZQuad(const glm::vec2& size);
};

} // namespace geometry
} // namespace gfx1993

#endif // GFX1993_QUAD_H
