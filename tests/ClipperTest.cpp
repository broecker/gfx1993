#include <GUnit.h>

#include <limits>

#include <glm/ext.hpp>
#include <glm/gtc/epsilon.hpp>

#include "Clipper.h"

namespace gfx1993 {

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::epsilonEqual;

template<class glm_type>
inline bool equal(const glm_type& a, const glm_type& b) { 
  return glm::epsilonEqual(a, b, std::numeric_limits<float>::min());
}

PointPrimitive makePoint(const vec3 &p) {
  VertexOut vout;
  vout.clipPosition = vec4(p, 1);
  return PointPrimitive(vout);
}

LinePrimitive makeLine(const vec3 &a, const vec3 &b) {
  VertexOut va;
  va.clipPosition = vec4(a, 1);
  va.texcoord = vec2(0);

  VertexOut vb;
  vb.clipPosition = vec4(b, 1);
  vb.texcoord = vec2(1);

  return LinePrimitive(va, vb);
}

TrianglePrimitive makeTriangle(const vec3 &a, const vec3 &b, const vec3 &c) {
  VertexOut va;
  va.clipPosition = vec4(a, 1);
  va.texcoord = vec2(0);

  VertexOut vb;
  vb.clipPosition = vec4(b, 1);
  vb.texcoord = vec2(1, 0);

  VertexOut vc;
  vc.clipPosition = vec4(c, 1);
  vc.texcoord = vec2(0, 1);

  return TrianglePrimitive(va, vb, vc);
}

GTEST("Clipper Plane Test") {
  SHOULD("Create from points") {
    Clipper::Plane plane(vec3(0, 0, 0), vec3(15, 0, 0), vec3(00, 20, 0));

    // Plane lies on origin.
    EXPECT(plane.distance(vec3(0, 0, 0)) == 0);
    // With 0,0,1 as up, as we defined it on the yz plane
    EXPECT(plane.getNormal() == vec3(0, 0, 1));
    EXPECT(plane.inFrontSpace(vec3(0, 0, 1)));
  }

  SHOULD("Calculate distances") {
    // Origin plane, with normal pointing up.
    Clipper::Plane plane(vec3(0, 1, 0), 0);

    // Points on plane.
    EXPECT(plane.distance(vec3(0, 0, 0)) == 0);
    EXPECT(plane.distance(vec3(2, 0, 5)) == 0);
    // Point one unit over plane.
    EXPECT(plane.distance(vec3(0, 1, 0)) == 1);
    // Point one unit below plane.
    EXPECT(plane.distance(vec3(0, -1, 0)) == -1);
  }

  SHOULD("Check frontspace") {
    // Origin plane, with normal pointing up.
    Clipper::Plane plane(vec3(0, 1, 0), 0);

    // Point on plane.
    EXPECT(plane.inFrontSpace(vec3(0, 0, 0)));
  }

  SHOULD("Check backspace") {
    // Origin plane, with normal pointing up.
    Clipper::Plane plane(vec3(0, 1, 0), 0);

    // Point 2 units below plane -- in back space.
    EXPECT(!plane.inFrontSpace(vec3(0, -2, 0)));
  }

  SHOULD("Check frontspace 2") {
    // Origin plane, with normal pointing up.
    Clipper::Plane plane(vec3(0, 1, 0), 0);

    // Point 2 units above plane along the normal -- in front space.
    EXPECT(plane.inFrontSpace(vec3(0, 2, 0)));
  }
};

GTEST("Clipper Test") {
  SHOULD("Create Ndc planes") {
    Clipper clipper;

    LinePrimitiveList lines;
    lines.push_back(makeLine(vec3(-2, 0, 0), vec3(2, 0, 0)));
    lines.push_back(makeLine(vec3(0, -2, 0), vec3(0, 2, 0)));
    lines.push_back(makeLine(vec3(0, 0, -2), vec3(0, 0, 2)));

    LinePrimitiveList clipped = clipper.clipLines(lines);

    ASSERT(clipped.size() == 3);
    EXPECT(equal(clipped[0].a.clipPosition, vec4(-1, 0, 0, 1)));
    EXPECT(equal(clipped[0].b.clipPosition, vec4(1, 0, 0, 1)));
    EXPECT(equal(clipped[1].a.clipPosition, vec4(0, -1, 0, 1)));
    EXPECT(equal(clipped[1].b.clipPosition, vec4(0, 1, 0, 1)));
    EXPECT(equal(clipped[2].a.clipPosition, vec4(0, 0, -1, 1)));
    EXPECT(equal(clipped[2].b.clipPosition, vec4(0, 0, 1, 1)));
  }

  SHOULD("Clip points against single plane") {
    // Origin plane, with normal pointing up.
    Clipper clipper(Clipper::Plane(vec3(0, 1, 0), 0));

    PointPrimitiveList points;
    points.push_back(makePoint(vec3(0, 2, 0)));  // In front space.
    points.push_back(makePoint(vec3(0, -2, 0))); // In back space.

    PointPrimitiveList clipped = clipper.clipPoints(points);

    EXPECT(clipped.size() == 1);
    // Keep the front space point.
    EXPECT(equal(clipped[0].p.clipPosition, vec4(0, 2, 0, 1)));
  }

  SHOULD("Clip points against single plane 2") {
    // XZ plane, normal pointing down at Y+2
    Clipper clipper(Clipper::Plane(vec3(0, -1, 0), -2));

    // Make 11 points that will cross the plane.
    PointPrimitiveList points;
    for (int i = 0; i <= 10; ++i) {
      points.push_back(makePoint(vec3(0, i, 0)));
    }

    PointPrimitiveList clipped = clipper.clipPoints(points);

    // Only the 3 points below the plane position of y = 2
    ASSERT(clipped.size() == 3);
    EXPECT(clipped[0].p.clipPosition.y == 0);
    EXPECT(clipped[1].p.clipPosition.y == 1);
    EXPECT(clipped[2].p.clipPosition.y == 2);
  }

  SHOULD("Clip points between two planes") {
    std::vector<Clipper::Plane> planes;
    // Bottom plane, at origin pointing up.
    planes.push_back(Clipper::Plane(vec3(0, 1, 0), 0));
    // Top plane, pointing down, 1 unit high.
    planes.push_back(Clipper::Plane(vec3(0, -1, 0), -1));
    Clipper clipper(planes);

    PointPrimitiveList points;
    points.push_back(makePoint(vec3(0, 0.5, 0))); // Inside
    points.push_back(makePoint(vec3(0, 2, 0)));   // Too high
    points.push_back(makePoint(vec3(0, -2, 0)));  // Too low
    points.push_back(makePoint(vec3(0, 0, 0)));   // Coplanar with bottom plane.

    PointPrimitiveList clipped = clipper.clipPoints(points);

    ASSERT(clipped.size() == 2);
    EXPECT(equal(clipped[0].p.clipPosition, vec4(0, 0.5, 0, 1)));
    EXPECT(equal(clipped[1].p.clipPosition, vec4(0, 0, 0, 1)));
  }

  SHOULD("Clip single line") {
    // Origin plane, with normal pointing up.
    Clipper clipper(Clipper::Plane(vec3(0, 1, 0), 0));

    LinePrimitiveList lines;
    lines.push_back(
        makeLine(vec3(0, 1, 0), vec3(0, -1, 0))); // Single line, top to bottom.
    lines.push_back(
        makeLine(vec3(0, -1, 0), vec3(0, 1, 0))); // Single bottom, to top.

    LinePrimitiveList clipped = clipper.clipLines(lines);

    ASSERT(clipped.size() == 2);

    EXPECT(clipped[0].a.clipPosition ==
          vec4(0, 1, 0, 1)); // First endpoint is unchanged.
    EXPECT(clipped[0].a.texcoord == vec2(0));
    EXPECT(clipped[0].b.clipPosition ==
          vec4(0, 0, 0, 1)); // Second endpoint got moved.
    EXPECT(clipped[0].b.texcoord ==
          vec2(0.5)); // Second parameters got interpolated.

    EXPECT(clipped[1].a.clipPosition ==
          vec4(0, 0, 0, 1)); // First endpoint moved to the plane.
    EXPECT(clipped[1].a.texcoord == vec2(0.5));
    EXPECT(clipped[1].b.clipPosition ==
          vec4(0, 1, 0, 1)); // Second endpoint is unchanged.
    EXPECT(clipped[1].b.texcoord == vec2(1));
  }

  SHOULD("Discard line in backspace") {
    // Origin plane, with normal pointing up.
    Clipper clipper(Clipper::Plane(vec3(0, 1, 0), 0));

    LinePrimitiveList lines;
    lines.push_back(makeLine(vec3(-2, -2, 0), vec3(5, -3, 2)));

    LinePrimitiveList clipped = clipper.clipLines(lines);

    EXPECT(clipped.empty());
  }

  SHOULD("Keep line in frontspace") {
    // Origin plane, with normal pointing up.
    Clipper clipper(Clipper::Plane(vec3(0, 1, 0), 0));

    LinePrimitiveList lines;
    lines.push_back(makeLine(vec3(-2, 1, 0), vec3(5, 0, 2)));

    LinePrimitiveList clipped = clipper.clipLines(lines);

    ASSERT(clipped.size() == 1);
    // Endpoints unchanged.
    EXPECT(clipped[0].a.clipPosition == vec4(-2, 1, 0, 1));
    EXPECT(clipped[0].a.texcoord == vec2(0));
    EXPECT(clipped[0].b.clipPosition == vec4(5, 0, 2, 1));
    EXPECT(clipped[0].b.texcoord == vec2(1));
  }

  SHOULD("Clip line against multiple planes") {
    std::vector<Clipper::Plane> planes;
    // Bottom XZ plane at Y-1; normal pointing up.
    planes.push_back(Clipper::Plane(vec3(0, 1, 0), -1));
    // Top XZ plane at Y+1; normal pointing down.
    planes.push_back(Clipper::Plane(vec3(0, -1, 0), -1));
    Clipper clipper(planes);

    LinePrimitiveList lines;
    lines.push_back(
        makeLine(vec3(0, 2, 0), vec3(0, -2, 0))); // Single line, top to bottom.
    lines.push_back(
        makeLine(vec3(0, -2, 0), vec3(0, 2, 0))); // Single bottom, to top.

    LinePrimitiveList clipped = clipper.clipLines(lines);

    ASSERT(clipped.size() == 2);
    EXPECT(equal(clipped[0].a.texcoord, vec2(0.25)));
    EXPECT(equal(clipped[0].b.clipPosition, vec4(0, -1, 0, 1)));
    EXPECT(equal(clipped[0].b.texcoord, vec2(0.75)));

    EXPECT(equal(clipped[1].a.clipPosition, vec4(0, -1, 0, 1)));
    EXPECT(equal(clipped[1].a.texcoord, vec2(0.25)));
    EXPECT(equal(clipped[1].b.clipPosition, vec4(0, 1, 0, 1)));
    EXPECT(equal(clipped[1].b.texcoord, vec2(0.75)));
  }

  SHOULD("Clip line against multiple planes 2") {
    std::vector<Clipper::Plane> planes;
    // Bottom XZ plane at origin; normal pointing up.
    planes.push_back(Clipper::Plane(vec3(0, 1, 0), 0));
    // Right YZ plane at X+2; normal point at X-1
    planes.push_back(Clipper::Plane(vec3(-1, 0, 0), -2));
    Clipper clipper(planes);

    LinePrimitiveList lines;
    lines.push_back(makeLine(vec3(-2, -2, 0), vec3(4, 4, 0)));

    LinePrimitiveList clipped = clipper.clipLines(lines);

    std::clog << "Clipped " << clipped.size() << std::endl;

    ASSERT(clipped.size() == 1);
    EXPECT(equal(clipped[0].a.clipPosition, vec4(0, 0, 0, 1)));
    EXPECT(equal(clipped[0].b.clipPosition, vec4(2, 2, 0, 1)));
  }

  SHOULD("Clip line against coplanar planes") {
    // Origin plane, with normal pointing up.
    Clipper clipper(Clipper::Plane(vec3(0, 1, 0), 0));

    LinePrimitiveList lines;
    lines.push_back(makeLine(vec3(5, 0, 3), vec3(-5, 0, 2)));

    LinePrimitiveList clipped = clipper.clipLines(lines);

    ASSERT(clipped.size() == 1);
    // Both endpoints are unchanged.
    EXPECT(equal(clipped[0].a.clipPosition, vec4(5, 0, 3, 1)));
    EXPECT(equal(clipped[0].a.texcoord, vec2(0)));
    EXPECT(equal(clipped[0].b.clipPosition, vec4(-5, 0, 2, 1)));
    EXPECT(equal(clipped[0].b.texcoord, vec2(1)));
  }

  SHOULD("Not clip triangle in frontspace") {
    // YZ plane at origin with normal at +Y
    Clipper clipper(Clipper::Plane(vec3(0, 1, 0), 0));

    TrianglePrimitiveList triangles;
    triangles.push_back(
        makeTriangle(vec3(0, 3, 0), vec3(-2, 1, 0), vec3(2, 1, 0)));

    TrianglePrimitiveList clipped = clipper.clipTriangles(triangles);

    ASSERT(clipped.size() == 1);
    EXPECT(equal(clipped[0].a.clipPosition, vec4(0, 3, 0, 1)));
    EXPECT(equal(clipped[0].b.clipPosition, vec4(-2, 1, 0, 1)));
    EXPECT(equal(clipped[0].c.clipPosition, vec4(2, 1, 0, 1)));
  }

  SHOULD("Discard triangle in backspace") {
    // YZ plane at origin with normal at -Y
    Clipper clipper(Clipper::Plane(vec3(0, -1, 0), 0));

    TrianglePrimitiveList triangles;
    triangles.push_back(
        makeTriangle(vec3(0, 3, 0), vec3(-2, 1, 0), vec3(2, 1, 0)));

    TrianglePrimitiveList clipped = clipper.clipTriangles(triangles);

    EXPECT(clipped.empty());
  }

  SHOULD("Not clip triangle against coplanar plane") {
    // XY plane with normal at +Z
    Clipper clipper(Clipper::Plane(vec3(0, 0, 1), 0));

    TrianglePrimitiveList triangles;
    // Triangle on XY plane.
    triangles.push_back(
        makeTriangle(vec3(0, 3, 0), vec3(-2, 1, 0), vec3(2, 1, 0)));

    TrianglePrimitiveList clipped = clipper.clipTriangles(triangles);

    ASSERT(clipped.size() == 1);
    EXPECT(equal(clipped[0].a.clipPosition, vec4(0, 3, 0, 1)));
    EXPECT(equal(clipped[0].b.clipPosition, vec4(-2, 1, 0, 1)));
    EXPECT(equal(clipped[0].c.clipPosition, vec4(2, 1, 0, 1)));
  }

  SHOULD("Clip triangle with a single point inside") {
    // XZ plane at two units over the origin.
    Clipper clipper(Clipper::Plane(vec3(0, 1, 0), 2));

    TrianglePrimitiveList triangles;
    // Triangle on XY plane.
    triangles.push_back(
        makeTriangle(vec3(0, 4, 0), vec3(-4, 0, 0), vec3(4, 0, 0)));

    TrianglePrimitiveList clipped = clipper.clipTriangles(triangles);

    ASSERT(clipped.size() == 1);
    EXPECT(equal(clipped[0].a.clipPosition,
          vec4(0, 4, 0, 1))); // Top vertex is unchanged.
    EXPECT(equal(clipped[0].b.clipPosition,
          vec4(-2, 2, 0, 1))); // Bottom left got moved.
    EXPECT(equal(clipped[0].c.clipPosition,
          vec4(2, 2, 0, 1))); // Bottom right got moved.
  }

  SHOULD("Clip triangle with a single point inside 2") {

    Clipper clipper(Clipper::Plane(vec3(-1, 0, 0), 2));

    TrianglePrimitiveList triangles;
    triangles.push_back(
        makeTriangle(vec3(0, 4, 0), vec3(-4, 0, 0), vec3(4, 0, 0)));

    TrianglePrimitiveList clipped = clipper.clipTriangles(triangles);

    ASSERT(clipped.size() == 1);
    EXPECT(equal(clipped[0].a.clipPosition,
          vec4(-2, 2, 0, 1))); // Top vertex got moved.
    EXPECT(equal(clipped[0].b.clipPosition,
          vec4(-4, 0, 0, 1))); // Bottom left is unchanged.
    EXPECT(equal(clipped[0].c.clipPosition,
          vec4(-2, 0, 0, 1))); // Bottom right got moved.
  }

  SHOULD("Clip triangle with a single point inside 3") {

    Clipper clipper(Clipper::Plane(vec3(1, 0, 0), 2));

    TrianglePrimitiveList triangles;
    triangles.push_back(
        makeTriangle(vec3(0, 4, 0), vec3(-4, 0, 0), vec3(4, 0, 0)));

    TrianglePrimitiveList clipped = clipper.clipTriangles(triangles);

    ASSERT(clipped.size() == 1);
    EXPECT(equal(clipped[0].a.clipPosition,
          vec4(2, 2, 0, 1))); // Top vertex got moved.
    EXPECT(equal(clipped[0].b.clipPosition,
          vec4(2, 0, 0, 1))); // Bottom left got moved.
    EXPECT(equal(clipped[0].c.clipPosition,
          vec4(4, 0, 0, 1))); // Bottom right is unchanged.
  }

  SHOULD("Clip triangle with two points inside") {
    Clipper clipper(Clipper::Plane(vec3(0, -1, 0), 0));

    TrianglePrimitiveList triangles;
    triangles.push_back(
        makeTriangle(vec3(2, -1, 0), vec3(-2, -1, 0), vec3(0, 1, 0)));

    auto clipped = clipper.clipTriangles(triangles);

    ASSERT(clipped.size() == 2);
    // Top position;
    EXPECT(equal(clipped[0].a.clipPosition, vec4(2, -1, 0, 1)));
    // Bottom left unchanged.
    EXPECT(equal(clipped[0].b.clipPosition, vec4(-2, -1, 0, 1))); 
    // Right position newly interpolated.
    EXPECT(equal(clipped[0].c.clipPosition, vec4(1, 0, 0, 1)));

    // Newly created triangle
    EXPECT(equal(clipped[1].a.clipPosition, vec4(1, 0, 0, 1)));
    EXPECT(equal(clipped[1].b.clipPosition, vec4(-2, -1, 0, 1)));
    EXPECT(equal(clipped[1].c.clipPosition, vec4(-1, 0, 0, 1)));
  }

  SHOULD("Clip triangle against multiple planes") {
    auto planes = std::vector<Clipper::Plane>{
        Clipper::Plane(vec3(0, 1, 0), 0), Clipper::Plane(vec3(1, 0, 0), 0),
        Clipper::Plane(vec3(-1, 0, 0),
                      -5) // This one should not interact with the triangle.
    };
    Clipper clipper(planes);

    TrianglePrimitiveList triangles;
    triangles.push_back(
        makeTriangle(vec3(4, -2, 0), vec3(-4, -2, 0), vec3(0, 2, 0)));

    auto clipped = clipper.clipTriangles(triangles);

    ASSERT(clipped.size() == 2);

    // Triangle is clipped by the vertical and horizontal planes.
    EXPECT(equal(clipped[0].a.clipPosition, vec4(2, 0, 0, 1)));
    EXPECT(equal(clipped[0].b.clipPosition, vec4(0, 0, 0, 1)));
    EXPECT(equal(clipped[0].c.clipPosition, vec4(0, 2, 0, 1)));

    // This creates a degenerate triangle.
    // TODO(mbroecker): Fix?
    EXPECT(equal(clipped[1].a.clipPosition, vec4(0, 0, 0, 1)));
    EXPECT(equal(clipped[1].b.clipPosition, vec4(0, 2, 0, 1)));
    EXPECT(equal(clipped[1].c.clipPosition, vec4(0, 2, 0, 1)));

  }
};

}  // namespace gfx1993
