#ifndef GFX1993_GEOMETRY_TERRAIN_INCLUDED
#define GFX1993_GEOMETRY_TERRAIN_INCLUDED

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "Geometry.h"
#include "Texture.h"
#include "Frustum.h"

namespace gfx1993 {

// Creates a simple pointfield based on a height map.
class PointField : public Geometry {
public:
  PointField(unsigned int width, unsigned int height);
  PointField(std::shared_ptr<HeightMap> heightMap);

  // Assigns each point a color whether it's inside or outside the frustum.
  void updatePoints(const Frustum& f);
private:
  std::shared_ptr<HeightMap> heightmap;
};

} // namespace gfx1993

#endif
