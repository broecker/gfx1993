#ifndef GFX1993_UTIL_BOUNDING_VOLUMES_INCLUDED
#define GFX1993_UTIL_BOUNDING_VOLUMES_INCLUDED

#include <glm/glm.hpp>
#include <limits>
#include <memory>


#include "Pipeline.h"
#include "Geometry.h"
#include "CubeGeometry.h"

namespace gfx1993 {

struct BoundingSphere {
  // In world coordinates.
  glm::vec3   center;
  float       radius;
};

// An alis-aligned bounding box with all coordinates in world space.
struct AABB {
  glm::vec3 min = glm::vec3(std::numeric_limits<glm::vec3::value_type>::max());
  glm::vec3 max = glm::vec3(std::numeric_limits<glm::vec3::value_type>::min());

  glm::vec3 getCenter() const { return (min + max) * 0.5f; }

  inline bool isInside(const glm::vec3& pt) const {
    return  pt.x >= min.x && pt.x <= max.x &&
            pt.y >= min.y && pt.y <= max.y &&
            pt.z >= min.z && pt.z <= max.z;
  }

  void extend(const glm::vec3& p);

  // Updates an existing geometry with these dimensions. Ideal for debug
  // drawing.
  void updateGeometry(Cube& cube);
};

// TODO(mbroecker) Extend with templates + generic stored data.
struct OctTree {
  AABB                      boundingBox;
  std::unique_ptr<OctTree>  children[8];

  // This currently stores vertices only but can be amended to stored any kind
  // of Geometry or game object. 
  VertexList        vertices;

  inline bool isLeafNode() const {
    for (int i = 0; i < 8; ++i) {
      if (children[i] != nullptr) {
        return false;
      }
    }
    return true;
  }
};

BoundingSphere fromGeometry(const Geometry& geo);

AABB fromVertices(const VertexList& vertices);

// Sample Octree implementation that splits a pointcloud recursively; i.e.
// builds the octtree top-down.
std::unique_ptr<OctTree> fromPointCloud(const VertexList& points,
                                        unsigned int maxVerticesPerNode);


} // namespace gfx1993

#endif // GFX1993_UTIL_BOUNDING_VOLUMES_INCLUDED
