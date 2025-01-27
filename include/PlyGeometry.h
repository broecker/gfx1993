#ifndef PLY_GEOMETRY_INCLUDED
#define PLY_GEOMETRY_INCLUDED

#include "Geometry.h"

#include <string>

namespace gfx1993 {

class PlyGeometry : public Geometry {
public:
  PlyGeometry();

  bool loadPly(const std::string &filename);

  // Centers the geometry without changing the transform.
  void center();

  inline glm::vec3 getCenter() const {
    return glm::vec3(0, boundingSphereRadius, 0);
  }

private:
  float boundingSphereRadius;
};

} // namespace gfx1993

#endif