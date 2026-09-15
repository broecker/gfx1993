#include <GUnit.h>

#include <glm/ext.hpp>
#include <glm/gtc/epsilon.hpp>

#include "BoundingVolumes.h"
#include "Pipeline.h"

namespace gfx1993 {

using glm::vec3;
using glm::vec4;

inline bool equalVec3(const vec3& a, const vec3& b, float eps = 1e-4f) {
  return glm::all(glm::epsilonEqual(a, b, eps));
}

// Checks that Cube::makeSolid()'s hardcoded triangles (the first triangle
// of each of its 6 faces) still have outward-facing normals given the
// cube's current vertex positions. See AABB::updateGeometry's comment for
// why this isn't automatic. Note: Cube::makeSolid()'s own inline comments
// ("-X", "+Z", ...) don't actually match which axis each triangle's face
// is on -- the expected directions below were derived from the real
// geometry (Cube::makeVertices()'s own vertex positions), not those
// comments.
bool hasOutwardWinding(const Cube& cube) {
  const auto& v = cube.getVertices();
  auto normal = [&](int a, int b, int c) {
    return glm::cross(vec3(v[b].position) - vec3(v[a].position),
                       vec3(v[c].position) - vec3(v[a].position));
  };
  return glm::dot(normal(0, 2, 1), vec3( 0, 1, 0)) > 0.f &&  // +Y
         glm::dot(normal(1, 6, 5), vec3( 0, 0,-1)) > 0.f &&  // -Z
         glm::dot(normal(2, 7, 6), vec3( 1, 0, 0)) > 0.f &&  // +X
         glm::dot(normal(3, 4, 7), vec3( 0, 0, 1)) > 0.f &&  // +Z
         glm::dot(normal(0, 5, 4), vec3(-1, 0, 0)) > 0.f &&  // -X
         glm::dot(normal(6, 4, 5), vec3( 0,-1, 0)) > 0.f;    // -Y
}

GTEST("AABB Test") {
  SHOULD("Collapse to a single point when extended once") {
    AABB bbox;
    bbox.extend(vec3(-3, -4, -5));

    EXPECT(bbox.min == vec3(-3, -4, -5));
    EXPECT(bbox.max == vec3(-3, -4, -5));
  }

  SHOULD("Extend correctly for bounds entirely in the negative octant") {
    // Regression test: AABB::max used to be default-initialized with
    // std::numeric_limits<float>::min(), which is the smallest *positive*
    // float (~1.18e-38), not the most negative one. That left max stuck
    // near zero instead of becoming negative whenever every extended point
    // was negative, i.e. whenever the box isn't symmetric around the origin.
    AABB bbox;
    bbox.extend(vec3(-5, -5, -5));
    bbox.extend(vec3(-1, -1, -1));

    EXPECT(bbox.min == vec3(-5, -5, -5));
    EXPECT(bbox.max == vec3(-1, -1, -1));
  }

  SHOULD("Extend correctly for bounds entirely in the positive octant") {
    AABB bbox;
    bbox.extend(vec3(1, 1, 1));
    bbox.extend(vec3(5, 5, 5));

    EXPECT(bbox.min == vec3(1, 1, 1));
    EXPECT(bbox.max == vec3(5, 5, 5));
  }

  SHOULD("Extend correctly for asymmetric bounds straddling the origin") {
    AABB bbox;
    bbox.extend(vec3(-10, 2, -9));
    bbox.extend(vec3(-1, 8, 3));

    EXPECT(bbox.min == vec3(-10, 2, -9));
    EXPECT(bbox.max == vec3(-1, 8, 3));
  }

  SHOULD("Compute the center of an asymmetric bounding box") {
    AABB bbox;
    bbox.extend(vec3(-10, -5, -9));
    bbox.extend(vec3(-2, -1, -3));

    EXPECT(bbox.getCenter() == vec3(-6, -3, -6));
  }

  SHOULD("Classify points inside an asymmetric, all-negative box") {
    AABB bbox;
    bbox.extend(vec3(-5, -5, -5));
    bbox.extend(vec3(-1, -1, -1));

    EXPECT(bbox.isInside(vec3(-3, -3, -3)) == true);
    EXPECT(bbox.isInside(vec3(0, 0, 0)) == false);
    EXPECT(bbox.isInside(vec3(-6, -3, -3)) == false);
  }

  SHOULD("Build an AABB from vertices entirely in the negative octant") {
    VertexList vertices;
    vertices.push_back(Vertex(vec4(-5, -5, -5, 1)));
    vertices.push_back(Vertex(vec4(-1, -1, -1, 1)));

    AABB bbox = fromVertices(vertices);

    EXPECT(bbox.min == vec3(-5, -5, -5));
    EXPECT(bbox.max == vec3(-1, -1, -1));
  }

  SHOULD("Build an AABB from vertices with asymmetric bounds") {
    VertexList vertices;
    vertices.push_back(Vertex(vec4(-8, 1, -6, 1)));
    vertices.push_back(Vertex(vec4(-2, 4, -1, 1)));

    AABB bbox = fromVertices(vertices);

    EXPECT(bbox.min == vec3(-8, 1, -6));
    EXPECT(bbox.max == vec3(-2, 4, -1));
  }

  SHOULD("Produce outward-facing winding when paired with Cube::makeSolid") {
    // Regression test: updateGeometry()'s corner-to-index assignment used
    // to run in the opposite rotational direction from Cube::makeVertices()'s
    // own layout, which Cube::makeSolid()'s hardcoded triangles are authored
    // against -- every face ended up wound inward. Only invisible before
    // because updateGeometry() was exclusively paired with Cube::makeLines(),
    // where winding direction doesn't matter.
    AABB bbox;
    bbox.extend(vec3(-1));
    bbox.extend(vec3(1));

    Cube cube = Cube::makeSolid();
    bbox.updateGeometry(cube);

    EXPECT(hasOutwardWinding(cube) == true);
  }
};

GTEST("OBB Test") {
  SHOULD("Match an equivalent AABB when transform is identity") {
    OBB obb(glm::mat4(1.f), vec3(2, 3, 4));

    AABB actual = AABB::fromOBB(obb);
    EXPECT(actual.min == vec3(-2, -3, -4));
    EXPECT(actual.max == vec3(2, 3, 4));
  }

  SHOULD("Classify points correctly for a rotated box") {
    // Local halfExtents (3,1,1) rotated 90 degrees about Y maps the long
    // local-X axis onto world -Z, so in world space this box spans
    // x in [-1,1], y in [-1,1], z in [-3,3].
    OBB obb(glm::rotate(glm::radians(90.f), vec3(0, 1, 0)), vec3(3, 1, 1));

    EXPECT(obb.isInside(vec3(0, 0, 2.5f)) == true);
    EXPECT(obb.isInside(vec3(0, 0, -2.9f)) == true);
    // A naive, rotation-ignoring check against the raw halfExtents (3,1,1)
    // would wrongly call this point inside (2.5 < 3); the real, rotated box
    // is only 1 unit wide along world X.
    EXPECT(obb.isInside(vec3(2.5f, 0, 0)) == false);
  }

  SHOULD("Compute a larger AABB for a rotated, non-axis-aligned box") {
    OBB obb(glm::rotate(glm::radians(90.f), vec3(0, 1, 0)), vec3(3, 1, 1));

    AABB bbox = AABB::fromOBB(obb);
    EXPECT(equalVec3(bbox.min, vec3(-1, -1, -3)));
    EXPECT(equalVec3(bbox.max, vec3(1, 1, 3)));
  }

  SHOULD("Produce outward-facing winding when paired with Cube::makeSolid") {
    OBB obb(glm::mat4(1.f), vec3(1));

    Cube cube = Cube::makeSolid();
    obb.updateGeometry(cube);

    EXPECT(hasOutwardWinding(cube) == true);
  }

  SHOULD("Preserve outward-facing winding after rotation") {
    OBB obb(glm::rotate(glm::radians(37.f), glm::normalize(vec3(0.3f, 1.f, 0.2f))),
            vec3(1));

    Cube cube = Cube::makeSolid();
    obb.updateGeometry(cube);

    EXPECT(hasOutwardWinding(cube) == true);
  }
};

}  // namespace gfx1993
