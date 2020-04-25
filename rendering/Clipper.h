#ifndef GFX1993_CLIPPER_H
#define GFX1993_CLIPPER_H

#include "Pipeline.h"

#include <glm/glm.hpp>
#include <vector>

namespace render {
class Clipper {
public:
  struct Plane {
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

  TrianglePrimitiveList
  clipTrianglesToNdc(TrianglePrimitiveList triangles) const;

private:
  std::vector<Plane> planes;
};

} // namespace render

#endif // GFX1993_CLIPPER_H
