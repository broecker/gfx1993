#ifndef GFX1993_QUAD_H
#define GFX1993_QUAD_H

#include "Geometry.h"

#include <glm/glm.hpp>

namespace geometry {

class Quad : public Geometry {
public:
  // Constructs a quad from 4 vertices. Assumes the following geometry:
  // a +---+ d
  //   | \ |
  // b +---+ c
  // The size will be [-1..1] along the XY axis and 0 on the z axis. It can be
  // used for screen-space rendering.
  explicit Quad(const glm::vec4 &color);
};

} // namespace geometry

#endif // GFX1993_QUAD_H
