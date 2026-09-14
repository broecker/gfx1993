#include <GUnit.h>

#include <limits>

#include <glm/ext.hpp>

#include "Depthbuffer.h"

namespace gfx1993 {

using glm::ivec2;
using glm::vec2;
using glm::vec3;

GTEST("Depthbuffer Test") {
  SHOULD("Report the size it was constructed with") {
    Depthbuffer db(4, 3);

    EXPECT(db.getWidth() == 4);
    EXPECT(db.getHeight() == 3);
  }

  SHOULD("Clear every depth to the given value") {
    Depthbuffer db(4, 4);
    db.clear(2.5f);

    for (unsigned int y = 0; y < db.getHeight(); ++y) {
      for (unsigned int x = 0; x < db.getWidth(); ++x) {
        EXPECT(db.getDepth(x, y) == 2.5f);
      }
    }
  }

  SHOULD("Default-clear to the farthest possible depth") {
    Depthbuffer db(2, 2);
    db.clear();

    EXPECT(db.getDepth(0, 0) == std::numeric_limits<float>::max());
  }

  SHOULD("Plot a depth value unconditionally") {
    Depthbuffer db(2, 2);
    db.clear(10.f);

    db.plot(ivec2(1, 0), 1.f);
    // Even a farther value should overwrite -- plot() is unconditional.
    db.plot(0, 1, 99.f);

    EXPECT(db.getDepth(1, 0) == 1.f);
    EXPECT(db.getDepth(ivec2(1, 0)) == 1.f);
    EXPECT(db.getDepth(0, 1) == 99.f);
  }

  SHOULD("Look up depth from relative coordinates, respecting width and height independently") {
    Depthbuffer db(4, 2);
    db.clear(0.f);
    db.plot(3, 1, 42.f);

    // Bottom-right texel of a 4x2 buffer.
    EXPECT(db.getDepth(vec2(0.99f, 0.99f)) == 42.f);
  }

  SHOULD("Conditionally plot only when the new depth is closer") {
    Depthbuffer db(2, 2);
    db.clear(5.f);

    EXPECT(db.conditionalPlot(0, 0, 1.f) == true);
    EXPECT(db.getDepth(0, 0) == 1.f);

    EXPECT(db.conditionalPlot(0, 0, 3.f) == false);
    EXPECT(db.getDepth(0, 0) == 1.f);
  }

  SHOULD("Truncate a vec3 position towards zero before conditional plotting") {
    Depthbuffer db(4, 4);
    db.clear(5.f);

    EXPECT(db.conditionalPlot(vec3(1.9f, 2.1f, 1.f)) == true);
    EXPECT(db.getDepth(1, 2) == 1.f);
  }

  SHOULD("Reject out-of-bounds conditional plots") {
    Depthbuffer db(2, 2);
    db.clear(5.f);

    EXPECT(db.conditionalPlot(-1, 0, 0.f) == false);
    EXPECT(db.conditionalPlot(0, -1, 0.f) == false);
    EXPECT(db.conditionalPlot(2, 0, 0.f) == false);
    EXPECT(db.conditionalPlot(0, 2, 0.f) == false);

    for (unsigned int y = 0; y < db.getHeight(); ++y) {
      for (unsigned int x = 0; x < db.getWidth(); ++x) {
        EXPECT(db.getDepth(x, y) == 5.f);
      }
    }
  }

  SHOULD("Report visibility by comparing against the stored depth") {
    Depthbuffer db(2, 2);
    db.clear(5.f);

    EXPECT(db.isVisible(0, 0, 1.f) == true);
    EXPECT(db.isVisible(0, 0, 5.f) == false);
    EXPECT(db.isVisible(0, 0, 9.f) == false);
    EXPECT(db.isVisible(ivec2(0, 0), 1.f) == true);
  }

  SHOULD("Track per-pixel write counts") {
    Depthbuffer db(2, 2);
    db.clear();

    EXPECT(db.getDepthWrites(0, 0) == 0);

    db.plot(0, 0, 5.f);
    EXPECT(db.getDepthWrites(0, 0) == 1);

    // A rejected conditional plot must not count as a write.
    EXPECT(db.conditionalPlot(0, 0, 10.f) == false);
    EXPECT(db.getDepthWrites(0, 0) == 1);

    // An accepted one does.
    EXPECT(db.conditionalPlot(0, 0, 1.f) == true);
    EXPECT(db.getDepthWrites(0, 0) == 2);

    EXPECT(db.getMaxDepthWrites() == 2);
  }

  SHOULD("Report the smallest depth value currently stored") {
    Depthbuffer db(2, 2);
    db.clear(10.f);
    db.plot(1, 1, 2.f);

    EXPECT(db.getMaxDepth() == 2.f);
  }
}

}  // namespace gfx1993
