#ifndef GFX1993_GEOMETRY_TERRAIN_INCLUDED
#define GFX1993_GEOMETRY_TERRAIN_INCLUDED

#include <glm/glm.hpp>
#include <memory>
#include <vector>

#include "Geometry.h"
#include "../rendering/Texture.h"
#include "../util/Frustum.h"

namespace gfx1993 {
namespace geometry {

// Creates a simple pointfield based on a height map.
class PointField : public geometry::Geometry {
public:
  PointField(unsigned int width, unsigned int height);
  PointField(std::shared_ptr<render::HeightMap> heightMap);

  // Assigns each point a color whether it's inside or outside the frustum.
  void updatePoints(const util::Frustum& f);
private:
  std::shared_ptr<render::HeightMap> heightmap;
};

} // namespace geometry
} // namespace gfx1993

#endif
