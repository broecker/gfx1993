#ifndef GFX1993_UTIL_BOUNDING_VOLUMES_INCLUDED
#define GFX1993_UTIL_BOUNDING_VOLUMES_INCLUDED

#include <array>
#include <glm/glm.hpp>
#include <limits>
#include <memory>


#include "Pipeline.h"
#include "Geometry.h"

namespace gfx1993 {

struct BoundingSphere {
  // In world coordinates.
  glm::vec3   center;
  float       radius;
};

class OBB;  // AABB::fromOBB below needs this; OBB is defined further down.

// An alis-aligned bounding box with all coordinates in world space.
struct AABB {
  glm::vec3 min = glm::vec3(std::numeric_limits<glm::vec3::value_type>::max());
  glm::vec3 max = glm::vec3(std::numeric_limits<glm::vec3::value_type>::lowest());

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

  // Computes the axis-aligned bounding box that contains an oriented box.
  static AABB fromOBB(const OBB& obb);
};

// An oriented bounding box: a box of size 2*halfExtents centered at the
// local origin, placed in world space by `transform`.
class OBB {
public:
  OBB() = default;
  OBB(const glm::mat4& transform, const glm::vec3& halfExtents)
      : halfExtents(halfExtents) {
    setTransform(transform);
  }

  // Sets the box's world-space placement and caches its inverse (used by
  // isInside()), so the inverse isn't recomputed on every query. The
  // transform's upper-left 3x3 may contain rotation and per-axis scale,
  // but its 3 columns must stay mutually orthogonal (no shear) for this to
  // represent a rectangular box rather than a general parallelepiped. If
  // composing rotation with non-uniform scale, compose as
  // `rotation * scale`, not `scale * rotation` (scale doesn't commute with
  // rotation, and the wrong order breaks orthogonality).
  void setTransform(const glm::mat4& t);

  const glm::mat4& getTransform() const { return transform; }

  glm::vec3 halfExtents = glm::vec3(0.5f);

  glm::vec3 getCenter() const { return glm::vec3(transform[3]); }

  // World-space half-extent vectors along each local axis, e.g.
  // center + axisX() reaches the middle of the local +X face. Already
  // incorporates any scale baked into transform's columns.
  glm::vec3 axisX() const { return glm::vec3(transform[0]) * halfExtents.x; }
  glm::vec3 axisY() const { return glm::vec3(transform[1]) * halfExtents.y; }
  glm::vec3 axisZ() const { return glm::vec3(transform[2]) * halfExtents.z; }

  // Same corner-to-index convention as AABB::updateGeometry (see that
  // function's definition for why the convention matters).
  std::array<glm::vec3, 8> getCorners() const;

  bool isInside(const glm::vec3& pt) const;

  // Updates an existing geometry with these dimensions. Ideal for debug
  // drawing.
  void updateGeometry(Cube& cube) const;

private:
  glm::mat4 transform        = glm::mat4(1.f);
  glm::mat4 inverseTransform = glm::mat4(1.f);
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
