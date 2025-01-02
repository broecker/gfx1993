#ifndef CUBE_GEOMETRY_INCLUDED
#define CUBE_GEOMETRY_INCLUDED

#include "Geometry.h"

namespace gfx1993 {
namespace geometry {

// A three-dimensional cube with solid faces.
class Cube : public Geometry {
public:
  static Cube makeSolid(const glm::vec3& sideLength = glm::vec3(1.f));
  static Cube makeLines(const glm::vec3& sideLength = glm::vec3(1.f));

private:
  Cube();
};

}  // namespace geometry
}  // namespace gfx1993
#endif
