#include <GUnit.h>

#include <limits>
#include <iostream>

#include <glm/ext.hpp>
#include <glm/gtc/epsilon.hpp>

#include "BoundingVolumes.h"
#include "Frustum.h"

namespace gfx1993 {

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

  SHOULD("Classify a small centered sphere as inside, not outside") {
    // Regression test: testIntersection(BoundingSphere) used to compare the
    // wrong way round (it returned OUTSIDE for any plane distance greater
    // than the radius, which is actually the "fully inside this plane"
    // case) and returned on the very first plane checked instead of testing
    // all 6. A small sphere sitting at the exact center of the frustum used
    // to come back OUTSIDE.
    Frustum frustum(glm::ortho(-1, 1, -1, 1, -1, 1));

    EXPECT(frustum.testIntersection(BoundingSphere{vec3(0), 0.5f}) == Frustum::INSIDE);
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

  SHOULD("Detect a rotated OBB intersecting a plane its local extents alone would miss") {
    // Same box, same position, as the next test below -- only the rotation
    // differs, and so does the correct answer. This proves the frustum test
    // actually accounts for orientation instead of only using halfExtents
    // as if the box were axis-aligned.
    Frustum frustum(glm::ortho(-1, 1, -1, 1, -1, 1));

    OBB box(glm::translate(vec3(1.36f, 0, 0)) * glm::rotate(glm::radians(45.f), vec3(0, 0, 1)),
            vec3(0.3f, 0.3f, 0.9f));
    EXPECT(frustum.testIntersection(box) == Frustum::INTERSECTING);
  }

  SHOULD("Classify the same unrotated box as fully outside") {
    Frustum frustum(glm::ortho(-1, 1, -1, 1, -1, 1));

    OBB box(glm::translate(vec3(1.36f, 0, 0)), vec3(0.3f, 0.3f, 0.9f));
    EXPECT(frustum.testIntersection(box) == Frustum::OUTSIDE);
  }
};


}  // namespace gfx1993