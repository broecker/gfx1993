#ifndef GFX1993_UTIL_FRUSTUM_INCLUDED
#define GFX1993_UTIL_FRUSTUM_INCLUDED

#include "../base/Pipeline.h"
#include "BoundingVolumes.h"

#include <glm/glm.hpp>

namespace gfx1993 {

using render::IndexList;
using render::VertexList;

namespace util {

// A frustum, for example for camera debugging and view culling.
// The 'geometry' should be drawn as lines. It has 8 vertices
// and 12 edges.
//
// The near plane consists of vertices A,B,C,D in CCW order.
// The far plane consists of vertices E,F,G,H in CCW order.
// E-A, B-F, C-G, D-H are connected.
//  E        H
//   +------+ --  A  D
//   |      |  -- +--+
//   |      |     |  |
//   |      |  -- +--+
//   +------+ --  B  C
//  F         G
// The frustum will be in /world/ coordinates.

class Frustum {
public:
  Frustum(const glm::mat4& projectionMatrix,
          const glm::mat4& viewMatrix=glm::mat4(1));
  
  // Updates the position of the frustum, keeping the projection constant. This
  // will also recalculate all planes used for clipping.
  void update(const glm::mat4& viewMatrix);

  // For debug line drawing.
  const IndexList& getIndices() const;
  const VertexList& getVertices() const;

  bool isInside(const glm::vec3& pt) const;

  enum IntersectionResult {
    OUTSIDE = 0,
    INSIDE = 1,
    INTERSECTING
  };

  IntersectionResult testIntersection(const BoundingSphere& sphere) const;

  IntersectionResult testIntersection(const AABB& boundingBox) const;

private:
  glm::mat4   projectionMatrix, viewMatrix;

  enum PlaneName {
    NEAR = 0,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    FAR,
    PLANES_COUNT
  };

  // Implicit notation for all planes.
  struct Plane {
    glm::vec3     normal;
    float         d;

    void set(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c);

    inline float distance(const glm::vec3& pt) const {
      return glm::dot(pt, normal) + d;
    }
  };
  
  Plane       planes[PLANES_COUNT];

  // Corner vertices for debug drawing.
  VertexList  vertices;
};

}  // namespace util
}  // namespace gfx1993


#endif // GFX1993_UTIL_FRUSTUM_INCLUDED