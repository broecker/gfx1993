#ifndef PLY_GEOMETRY_INCLUDED
#define PLY_GEOMETRY_INCLUDED

#include "Geometry.h"

#include <string>

namespace gfx1993 {
namespace geometry {

class PlyGeometry : public Geometry {
public:
  PlyGeometry();

  bool loadPly(const std::string &filename);

  // Centers the geometry without changing the transform.
  void center();

private:
  float boundingSphereRadius;
};

} // namespace geometry
} // namespace gfx1993

#endif