#ifndef GFX1993_UTIL_BOUNDING_VOLUMES_INCLUDED
#define GFX1993_UTIL_BOUNDING_VOLUMES_INCLUDED

#include <glm/glm.hpp>

#include "../geometry/Geometry.h"

namespace gfx1993 {
namespace util {

struct BoundingSphere {
  // In world coordinates.
  glm::vec3   center;
  float       radius;
};

BoundingSphere fromGeometry(const geometry::Geometry& geo);

} // namespace util 
} // namespace gfx1993

#endif // GFX1993_UTIL_BOUNDING_VOLUMES_INCLUDED
