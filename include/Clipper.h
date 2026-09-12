#ifndef GFX1993_CLIPPER_H
#define GFX1993_CLIPPER_H

#include "Pipeline.h"

#include <glm/glm.hpp>
#include <vector>

namespace gfx1993 {

class ThreadPool;

class Clipper {
public:
  struct Plane {
    // Stores the plane as normal and *negative* distance so we cause a dot
    // product to calculate signed distance.
    glm::vec4 plane;

    // Creates a plane given by a normal and a distance to the origin along that
    // normal.
    Plane(const glm::vec3 &normal, float distance)
        : plane(normalize(normal), -distance) {}

    // Creates a plane from 3 given counter-clockwise points.
    Plane(const glm::vec3 &a, const glm::vec3 &b, const glm::vec3 &c);

    inline bool inFrontSpace(const glm::vec3 &point) const {
      return distance(point) >= 0;
    }

    inline float distance(const glm::vec3 &point) const {
      return distance(glm::vec4(point, 1.f));
    }

    inline float distance(const glm::vec4 &pt) const { return dot(pt, plane); }

    inline glm::vec3 getNormal() const { return glm::vec3(plane); }
  };

  explicit Clipper(const Plane &plane);
  explicit Clipper(const std::vector<Plane> &planes);
  explicit Clipper();

  PointPrimitiveList clipPoints(const PointPrimitiveList &points) const;

  PointPrimitiveList clipPointsToNdc(const PointPrimitiveList &points) const;

  LinePrimitiveList clipLines(const LinePrimitiveList &lines) const;

  TrianglePrimitiveList clipTriangles(TrianglePrimitiveList triangles) const;

  // Clips triangles plane-by-plane, parallelizing the per-plane work across
  // `pool`. The caller owns the pool (typically the owning Rasterizer's
  // single shared ThreadPool) so that all of a Rasterizer's parallel work
  // shares one set of worker threads.
  TrianglePrimitiveList
  clipTrianglesToNdc(TrianglePrimitiveList triangles,
                     ThreadPool &pool) const;

  void toggleDebug() const { debugColorClips = !debugColorClips; }

private:
  std::vector<Plane> planes;

  // If set, colors created triangles in yellow;
  mutable bool debugColorClips = false;
  glm::vec4 debugClipColor = glm::vec4(1,0,1,1);
};

} // namespace gfx1993

#endif // GFX1993_CLIPPER_H
