#include <GUnit.h>

#include <limits>
#include <iostream>

#include <glm/ext.hpp>
#include <glm/gtc/epsilon.hpp>

#include "BoundingVolumes.h"
#include "Frustum.h"

namespace gfx1993 {
namespace util {

using glm::vec3;
using glm::vec4;

GTEST("Frustum Test") {
  SHOULD("Create perspective frustum") {
    Frustum frustum(glm::perspective(30.f, 1.f, 1.f, 100.f)); 
    // 8 vertices for the frustum corners +
    // 6 planes with 1 normal (2 vertices each) = 
    // 8 + 12 = 20 vertices total.
    EXPECT(frustum.getVertices().size() == 20);
  }

  SHOULD("Create ortho frustum") {
    Frustum frustum(glm::ortho(-1, 1, -1, 1, -1, 1));

    EXPECT(frustum.getVertices().size() == 20);
    // Check the 8 corner vertices.
    EXPECT(frustum.getVertices()[0].position == vec4(-1, 1, 1,1));
    EXPECT(frustum.getVertices()[1].position == vec4(-1,-1, 1,1));
    EXPECT(frustum.getVertices()[2].position == vec4( 1,-1, 1,1));
    EXPECT(frustum.getVertices()[3].position == vec4( 1, 1, 1,1));
    EXPECT(frustum.getVertices()[4].position == vec4(-1, 1,-1,1));
    EXPECT(frustum.getVertices()[5].position == vec4(-1,-1,-1,1));
    EXPECT(frustum.getVertices()[6].position == vec4( 1,-1,-1,1));
    EXPECT(frustum.getVertices()[7].position == vec4( 1, 1,-1,1));
  }

  SHOULD("Handle point intersections") {
    Frustum frustum(glm::ortho(-1, 1, -1, 1, -1, 1));

    EXPECT(frustum.isInside(vec3(-2, 0, 0)) == false);
    EXPECT(frustum.isInside(vec3(0)) == true);

    EXPECT(frustum.isInside(vec3(0, -1.1, 0)) == false);
  }

  SHOULD("Classify coplanar points as inside") {
    Frustum frustum(glm::ortho(-1, 1, -1, 1, -1, 1));

    EXPECT(frustum.isInside(vec3(0, 0, 1)) == true);
  }

  SHOULD("Handle sphere intersections") {
    Frustum frustum(glm::ortho(-1, 1, -1, 1, -1, 1));

    EXPECT(frustum.testIntersection({vec3(-2, 0, 0), 0.5 }) == Frustum::OUTSIDE);

    // Intersecting from the outside.
    EXPECT(frustum.testIntersection({vec3(-1.5,0,0), 1.f}) == Frustum::INTERSECTING);

    // Intersecting from the inside.
    EXPECT(frustum.testIntersection({vec3(-0.2,0,0), 1.f}) == Frustum::INTERSECTING);

    EXPECT(frustum.testIntersection({vec3(0), 1.f}) == Frustum::INSIDE);
  }

  SHOULD("Detect completely outside bounding boxes") {
    Frustum frustum(glm::ortho(-1, 1, -1, 1, -1, 1));

    AABB bbox{vec3(-3), vec3(-2)};
    EXPECT(frustum.testIntersection(bbox) == Frustum::OUTSIDE);
  }

  SHOULD("Detect inside bounding boxes") {
    Frustum frustum(glm::ortho(-1, 1, -1, 1, -1, 1));

    AABB bbox{vec3(-0.5), vec3(0.5)};
    EXPECT(frustum.testIntersection(bbox) == Frustum::INSIDE);
  }

  SHOULD("Detect intersecting bounding boxes") {
    Frustum frustum(glm::ortho(-1, 1, -1, 1, -1, 1));

    AABB bbox{vec3(-1.5), vec3(0.5f)};
    EXPECT(frustum.testIntersection(bbox) == Frustum::INTERSECTING);
  }

  SHOULD("Detect frustum inside bounding box") {
    Frustum frustum(glm::ortho(-1, 1, -1, 1, -1, 1));

    AABB bbox{vec3(-1.5), vec3(1.5)};
    EXPECT(frustum.testIntersection(bbox) == Frustum::INTERSECTING);
  }
  
};


}  // namespace util
}  // namespace gfx1993