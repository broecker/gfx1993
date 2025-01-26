#ifndef GFX1993_GEOMETRY_SPHERE_INCLUDED
#define GFX1993_GEOMETRY_SPHERE_INCLUDED

#include "Geometry.h"

namespace gfx1993 {
namespace geometry {

class Sphere : public Geometry {
public:
  Sphere(float radius, unsigned int latitudes, unsigned int longitudes);
};

} // namespace geometry
} // namespace gfx1993


#endif  // GFX1993_GEOMETRY_SPHERE_INCLUDED