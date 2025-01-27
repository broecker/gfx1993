#include "Terrain.h"

namespace gfx1993 {

using glm::vec4;

PointField::PointField(std::shared_ptr<HeightMap> heightMap) :
  heightmap(heightMap) {
    for (int x = 0; x < heightMap->getWidth(); x++) {
      for (int z = 0; z < heightMap->getHeight(); z++) {
        Vertex v;
        v.color = glm::vec4(1,0,1,1);
        v.normal = glm::vec3(0);
        v.texcoord = glm::vec2(static_cast<float>(x)/heightMap->getWidth(),
                               static_cast<float>(z)/heightMap->getHeight());

        float y = heightmap->getTexel(glm::ivec2(x, z));
        v.position = glm::vec4(x, y, z, 1);

        vertices.push_back(v);
        indices.push_back(vertices.size()-1);
      }
    }
    assert(indices.size() == vertices.size());
  }

// Assigns each point a color whether it's inside or outside the frustum.
void PointField::updatePoints(const Frustum& f) {
  for (Vertex& v : vertices) {
    vec4 worldSpacePosition = transform * v.position;
    if (f.isInside(glm::vec3(worldSpacePosition))) {
      v.color = glm::vec4(0,1,0,1);
    } else {
      v.color = glm::vec4(1,0,0,1);
    }
  }
}

} // namespace gfx1993
