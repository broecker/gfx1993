#include "Clipper.h"
#include <algorithm>
#include <iostream>
#include <map>

#include <deque>
#include <glm/gtx/io.hpp>

namespace render {

Clipper::Plane::Plane(const glm::vec3 &a, const glm::vec3 &b,
                      const glm::vec3 &c) {
  auto r = glm::normalize(b - a);
  auto s = glm::normalize(c - a);
  auto normal = glm::normalize(glm::cross(r, s));
  float distance = glm::dot(normal, a);

  plane = glm::vec4(normal, -distance);
}

Clipper::Clipper(const Clipper::Plane &plane) { planes.push_back(plane); }

Clipper::Clipper(const std::vector<Plane> &clipPlanes) : planes(clipPlanes) {}

Clipper::Clipper() {
  using glm::vec3;
  using glm::vec4;

  // The six clip planes in NDC.
  planes.push_back(Plane(vec3(1.f, 0.f, 0.f), -1.f));
  planes.push_back(Plane(vec3(-1.f, 0, 0), -1.f));
  planes.push_back(Plane(vec3(0, 1.f, 0), -1.f));
  planes.push_back(Plane(vec3(0, -1.f, 0), -1.f));
  planes.push_back(Plane(vec3(0, 0, 1.f), -1.f));
  planes.push_back(Plane(vec3(0.f, 0.f, -1.f), -1.f));
}

PointPrimitiveList Clipper::clipPoints(const PointPrimitiveList &points) const {
  PointPrimitiveList clipped;
  clipped.reserve(points.size());

  for (const PointPrimitive &point : points) {
    bool keep = true;
    for (const Plane &p : planes) {
      if (!p.inFrontSpace(point.p.clipPosition)) {
        keep = false;
        break;
      }
    }
    if (keep) {
      clipped.push_back(point);
    }
  }
  return clipped;
}

inline static bool insideNDC(const glm::vec4 &clipCoords) {
  return clipCoords.x >= -clipCoords.w && clipCoords.x <= clipCoords.w &&
         clipCoords.y >= -clipCoords.w && clipCoords.y <= clipCoords.w &&
         clipCoords.z >= -clipCoords.w && clipCoords.z <= clipCoords.w &&
         clipCoords.w > 0;
}

PointPrimitiveList
Clipper::clipPointsToNdc(const PointPrimitiveList &points) const {
  PointPrimitiveList clipped;
  clipped.reserve(points.size());

  for (const PointPrimitive &p : points) {
    if (insideNDC(p.p.clipPosition)) {
      clipped.push_back(p);
    }
  }
  return clipped;
}

LinePrimitiveList
Clipper::clipLines(const render::LinePrimitiveList &lines) const {
  using glm::vec4;

  LinePrimitiveList clipped;

  for (LinePrimitive line : lines) {
    bool keep = true;
    for (const Clipper::Plane &plane : planes) {
      // Signed distances from the clipping planes to both line end points.
      float d0 = plane.distance(line.a.clipPosition);
      float d1 = plane.distance(line.b.clipPosition);

      // Both are outside -- discard.
      if (d0 < 0.f && d1 < 0.f) {
        keep = false;
        break;
      }
      // Both are inside -- no clipping needed.
      if (d0 >= 0.f && d1 >= 0.f) {
        continue;
      }

      // One is outside, the other inside -- calculate intersection and clip.
      float t = d0 / (d0 - d1);
      // a is inside, b outside
      if (d0 >= 0.f)
        line.b = lerp(line.a, line.b, t);
      // a is outside, b inside
      else
        line.a = lerp(line.a, line.b, t);
    }
    if (keep) {
      clipped.push_back(line);
    }
  }
  return clipped;
}

// Clips the edge ab against the given plane and returns the point on the plane.
static VertexOut clipEdge(const VertexOut &a, const VertexOut &b,
                          const Clipper::Plane &plane) {
  float da = plane.distance(a.clipPosition);
  float db = plane.distance(b.clipPosition);

  // One is outside, the other inside -- calculate intersection and clip.
  float t = da / (da - db);
  return lerp(a, b, t);
}

static void setColor(render::TrianglePrimitive &triangle,
                     const glm::vec4 &color) {
  triangle.a.color = color;
  triangle.b.color = color;
  triangle.c.color = color;
}

TrianglePrimitiveList
Clipper::clipTriangles(render::TrianglePrimitiveList triangles) const {
  TrianglePrimitiveList clipped;
  clipped.reserve(triangles.size());

  const static size_t MAX_TRI_COUNT = 1000;

  bool colorClips = true;

  for (size_t i = 0; i < triangles.size() && i < MAX_TRI_COUNT; ++i) {
    // Triangles might change during iteration, as new triangles are created by
    // clipping.
    TrianglePrimitive triangle = triangles[i];

    std::vector<glm::vec4> clipPositions;
    clipPositions.push_back(triangle.a.clipPosition);
    clipPositions.push_back(triangle.b.clipPosition);
    clipPositions.push_back(triangle.c.clipPosition);

    /*
    if (std::all_of(clipPositions.begin(), clipPositions.end(),
                    [](const glm::vec4 &v) { return v.w <= 0.f; })) {
      // All behind the w=0 axis -- discard;
      continue;
    }
    */

    bool keep = true;
    for (const Plane &plane : planes) {
      bool aInFrontSpace = plane.inFrontSpace(triangle.a.clipPosition);
      bool bInFrontSpace = plane.inFrontSpace(triangle.b.clipPosition);
      bool cInFrontSpace = plane.inFrontSpace(triangle.c.clipPosition);

      // All points visible, keep going.
      if (aInFrontSpace && bInFrontSpace && cInFrontSpace) {
        continue;
      }

      // All points invisible, discard.
      if (!aInFrontSpace && !bInFrontSpace && !cInFrontSpace) {
        keep = false;
        break;
      }

      // Two possible cases: either one inside, two outside; or two inside, one
      // outside.
      float da = plane.distance(triangle.a.clipPosition);
      float db = plane.distance(triangle.b.clipPosition);
      float dc = plane.distance(triangle.c.clipPosition);

      // a inside; b,c outside;
      if (da >= 0 && db < 0 && dc < 0) {
        triangle.b = clipEdge(triangle.a, triangle.b, plane);
        triangle.c = clipEdge(triangle.a, triangle.c, plane);
        continue;
      }

      // b inside; a,c outside;
      if (db >= 0 && da < 0 && dc < 0) {
        triangle.a = clipEdge(triangle.a, triangle.b, plane);
        triangle.c = clipEdge(triangle.c, triangle.b, plane);
        continue;
      }

      // c inside, a,b outside
      if (dc >= 0 && da < 0 && db < 0) {
        triangle.a = clipEdge(triangle.c, triangle.a, plane);
        triangle.b = clipEdge(triangle.c, triangle.b, plane);
        continue;
      }

      if (da >= 0 && db >= 0) {
        VertexOut p = clipEdge(triangle.a, triangle.c, plane);
        VertexOut q = clipEdge(triangle.b, triangle.c, plane);
        // Truncate current triangle, add new one to the list.
        triangle.c = p;

        TrianglePrimitive t(p, triangle.b, q);
        if (colorClips) {
          setColor(t, glm::vec4(1, 0, 1, 1));
        }
        triangles.push_back(t);
        continue;
      }

      if (da >= 0 && dc >= 0) {
        VertexOut p = clipEdge(triangle.a, triangle.b, plane);
        VertexOut q = clipEdge(triangle.c, triangle.b, plane);
        triangle.b = p;

        TrianglePrimitive t(p, q, triangle.c);
        if (colorClips) {
          setColor(t, glm::vec4(1, 1, 0, 1));
        }
        triangles.push_back(t);
        continue;
      }

      if (db >= 0 && dc >= 0) {
        VertexOut p = clipEdge(triangle.b, triangle.a, plane);
        VertexOut q = clipEdge(triangle.c, triangle.a, plane);
        triangle.a = p;

        TrianglePrimitive t(p, triangle.c, q);
        if (colorClips) {
          setColor(t, glm::vec4(0, 1, 1, 1));
        }
        triangles.push_back(t);
        continue;
      }
    }

    if (keep) {
      clipped.push_back(triangle);
    }
  }
  return clipped;
}

// Selects the endpoint for a given triangle edge.
static const VertexOut &getTriangleEdgePoint(const TrianglePrimitive &triangle,
                                             int i) {
  switch (i) {
  case 0:
    return triangle.a;
  case 1:
    return triangle.b;
  case 2:
    return triangle.c;
  case 3:
    // This allows connecting the last edge to the beginning.
    return triangle.a;

  default:
    // This is bad.
    std::cerr << "Bad triangle edge selection." << std::endl;
    return VertexOut();
  }
}

// Sutherland-Hodgeman triangle clipping.
// Based on:
// https://www.flipcode.com/archives/Real-time_3D_Clipping_Sutherland-Hodgeman.shtml
static TrianglePrimitiveList clipTriangle(const TrianglePrimitive &triangle,
                                          const Clipper::Plane &plane) {
  // Contains clipped coordinates.
  std::vector<VertexOut> clipped;
  for (int i = 0; i < 3; ++i) {
    const VertexOut &v1 = getTriangleEdgePoint(triangle, i);
    const VertexOut &v2 = getTriangleEdgePoint(triangle, i + 1);

    float d1 = plane.distance(v1.clipPosition);
    float d2 = plane.distance(v2.clipPosition);

    // v1 inside, v2 outside
    if (d1 >= 0 && d2 < 0)
      clipped.push_back(clipEdge(v1, v2, plane));

    // v1 outside, v1 inside
    if (d1 < 0 && d2 >= 0) {
      clipped.push_back(clipEdge(v1, v2, plane));
      clipped.push_back(v2);
    }

    // v1 and v2 inside
    if (d1 >= 0 && d2 >= 0)
      clipped.push_back(v2);

    // v1 and v2 outside -- don't do anything
  }

  // Reconstruct the triangle(s) from the clipped edges. The output can be
  // either zero, one or two triangles.
  TrianglePrimitiveList output;
  if (clipped.size() >= 3) {
    output.push_back(TrianglePrimitive(clipped[0], clipped[1], clipped[2]));
  }
  if (clipped.size() == 4) {
    auto t = TrianglePrimitive(clipped[2], clipped[3], clipped[0]);
    setColor(t, glm::vec4(1, 1, 0, 1));
    output.push_back(t);
  }
  return output;
}

TrianglePrimitiveList Clipper::clipTrianglesToNdc(
    const TrianglePrimitiveList &clipspaceTriangles) const {
  TrianglePrimitiveList clipped = clipspaceTriangles;
  for (const auto &plane : planes) {
    TrianglePrimitiveList temp;

    for (const auto &t : clipped) {
      auto result = clipTriangle(t, plane);
      temp.insert(temp.end(), result.begin(), result.end());
    }
    clipped = temp;
  }
  return clipped;
}

} // namespace render
