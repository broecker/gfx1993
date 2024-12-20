#ifndef CUBE_GEOMETRY_INCLUDED
#define CUBE_GEOMETRY_INCLUDED

#include "Geometry.h"

namespace gfx1993 {
namespace geometry {

// A three-dimensional cube with solid faces.
class CubeGeometry : public Geometry {
public:
  explicit CubeGeometry(const glm::vec3 &sideLength);
};

}  // namespace geometry
}  // namespace gfx1993
#endif
