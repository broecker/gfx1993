#ifndef GFX1993_UTIL_FRUSTUM_INCLUDED
#define GFX1993_UTIL_FRUSTUM_INCLUDED

#include "Geometry.h"

#include <glm/glm.hpp>

namespace gfx1993 {
namespace geometry {

// A frustum, for example for camera debugging and view culling.
// The 'geometry' should be drawn as lines. It has 8 vertices
// and 12 edges.
//
// The near plane consists of vertices A,B,C,D in CCW order.
// The far plane consists of vertices E,F,G,H in CCW order.
// E-A, B-F, C-G, D-H are connected.
//  E        H
//   +------+ --  A  D
//   |      |  -- +--+
//   |      |     |  |
//   |      |  -- +--+
//   +------+ --  B  C
//  F         G
// The frustum will be in /world/ coordinates.

class Frustum : public Geometry {
public:
  Frustum(const glm::mat4& projectionMatrix,
          const glm::mat4& viewMatrix=glm::mat4(1));
  
  void update(const glm::mat4& viewMatrix);

private:
  glm::mat4   projectionMatrix, viewMatrix;
};

}  // namespace geometry
}  // namespace gfx1993


#endif // GFX1993_UTIL_FRUSTUM_INCLUDED