#ifndef GFX1993_UTIL_BOUNDING_VOLUMES_INCLUDED
#define GFX1993_UTIL_BOUNDING_VOLUMES_INCLUDED

#include <glm/glm.hpp>
#include <memory>

#include "../base/Pipeline.h"
#include "../geometry/Geometry.h"

namespace gfx1993 {
namespace util {

struct BoundingSphere {
  // In world coordinates.
  glm::vec3   center;
  float       radius;
};

// An alis-aligned bounding box with all coordinates in world space.
struct AABB {
  glm::vec3   min, max;

  glm::vec3 getCenter() const { return (min + max) * 0.5f; }

  inline bool isInside(const glm::vec3& pt) const {
    return  pt.x >= min.x && pt.x <= max.x &&
            pt.y >= min.y && pt.y <= max.y &&
            pt.z >= min.z && pt.z <= max.z;
  }
};

// TODO(mbroecker) Extend with templates + generic stored data.
struct OctTree {
  AABB                      boundingBox;
  std::unique_ptr<OctTree>  children[8];

  // This currently stores vertices only but can be amended to stored any kind
  // of Geometry or game object. 
  render::VertexList        vertices;

  inline bool isLeafNode() const {
    for (int i = 0; i < 8; ++i) {
      if (children[i] != nullptr) {
        return false;
      }
    }
    return true;
  }
};

BoundingSphere fromGeometry(const geometry::Geometry& geo);

AABB fromVertices(const render::VertexList& vertices);

// Sample Octree implementation that splits a pointcloud recursively; i.e.
// builds the octtree top-down.
std::unique_ptr<OctTree> fromPointCloud(const render::VertexList& points,
                                        unsigned int maxVerticesPerNode);


} // namespace util 
} // namespace gfx1993

#endif // GFX1993_UTIL_BOUNDING_VOLUMES_INCLUDED
