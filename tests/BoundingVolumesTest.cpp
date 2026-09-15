#include <GUnit.h>

#include <glm/ext.hpp>

#include "BoundingVolumes.h"
#include "Pipeline.h"

namespace gfx1993 {

using glm::vec3;
using glm::vec4;

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
};

}  // namespace gfx1993
