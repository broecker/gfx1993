#include <GUnit.h>

#include <glm/ext.hpp>

#include "Framebuffer.h"

namespace gfx1993 {

using glm::ivec2;
using glm::vec2;
using glm::vec4;

GTEST("Framebuffer Test") {
  SHOULD("Report the size it was constructed with") {
    Framebuffer fb(4, 3);

    EXPECT(fb.getWidth() == 4);
    EXPECT(fb.getHeight() == 3);
  }

  SHOULD("Clear every pixel to the given color") {
    Framebuffer fb(4, 4);
    fb.clear(vec4(0.25f, 0.5f, 0.75f, 1.f));

    for (unsigned int y = 0; y < fb.getHeight(); ++y) {
      for (unsigned int x = 0; x < fb.getWidth(); ++x) {
        EXPECT(fb.getPixel(x, y) == vec4(0.25f, 0.5f, 0.75f, 1.f));
      }
    }
  }

  SHOULD("Plot a pixel at the given coordinates") {
    Framebuffer fb(4, 4);
    fb.clear(vec4(0));

    fb.plot(ivec2(1, 2), vec4(1, 0, 0, 1));

    EXPECT(fb.getPixel(1, 2) == vec4(1, 0, 0, 1));
    EXPECT(fb.getPixel(ivec2(1, 2)) == vec4(1, 0, 0, 1));
    // Neighboring pixels should be untouched.
    EXPECT(fb.getPixel(0, 2) == vec4(0));
    EXPECT(fb.getPixel(1, 1) == vec4(0));
  }

  SHOULD("Ignore out-of-bounds plots instead of crashing") {
    Framebuffer fb(4, 4);
    fb.clear(vec4(0));

    fb.plot(4, 0, vec4(1));
    fb.plot(0, 4, vec4(1));

    for (unsigned int y = 0; y < fb.getHeight(); ++y) {
      for (unsigned int x = 0; x < fb.getWidth(); ++x) {
        EXPECT(fb.getPixel(x, y) == vec4(0));
      }
    }
  }

  SHOULD("Look up the nearest pixel from relative coordinates") {
    Framebuffer fb(2, 2);
    fb.plot(0, 0, vec4(1, 0, 0, 1));
    fb.plot(1, 0, vec4(0, 1, 0, 1));
    fb.plot(0, 1, vec4(0, 0, 1, 1));
    fb.plot(1, 1, vec4(1, 1, 0, 1));

    EXPECT(fb.getPixel(vec2(0.f, 0.f)) == vec4(1, 0, 0, 1));
    EXPECT(fb.getPixel(vec2(1.f, 0.f)) == vec4(0, 1, 0, 1));
    EXPECT(fb.getPixel(vec2(0.f, 1.f)) == vec4(0, 0, 1, 1));
    EXPECT(fb.getPixel(vec2(1.f, 1.f)) == vec4(1, 1, 0, 1));
  }

  SHOULD("Clamp out-of-range relative coordinates instead of crashing") {
    Framebuffer fb(2, 2);
    fb.plot(0, 0, vec4(1, 0, 0, 1));
    fb.plot(1, 1, vec4(1, 1, 0, 1));

    EXPECT(fb.getPixel(vec2(-1.f, -1.f)) == vec4(1, 0, 0, 1));
    EXPECT(fb.getPixel(vec2(2.f, 2.f)) == vec4(1, 1, 0, 1));
  }

  SHOULD("Expose the raw pixel data matching getPixel") {
    Framebuffer fb(2, 2);
    fb.plot(1, 1, vec4(0.1f, 0.2f, 0.3f, 0.4f));

    const vec4* pixels = fb.getPixels();
    EXPECT(pixels[1 + 1 * fb.getWidth()] == vec4(0.1f, 0.2f, 0.3f, 0.4f));
  }

  SHOULD("Convert to an interleaved 8-bit RGBA buffer") {
    Framebuffer fb(2, 1);
    fb.plot(0, 0, vec4(1.f, 0.f, 0.5f, 0.f));
    fb.plot(1, 0, vec4(0.f, 1.f, 0.f, 1.f));

    std::vector<uint8_t> buffer = fb.getUint8RgbaBuffer();

    EXPECT(buffer.size() == 2u * 4u);
    EXPECT(static_cast<int>(buffer[0]) == 255);
    EXPECT(static_cast<int>(buffer[1]) == 0);
    EXPECT(static_cast<int>(buffer[2]) == static_cast<int>(0.5f * 255));
    EXPECT(static_cast<int>(buffer[3]) == 0);

    EXPECT(static_cast<int>(buffer[4]) == 0);
    EXPECT(static_cast<int>(buffer[5]) == 255);
    EXPECT(static_cast<int>(buffer[6]) == 0);
    EXPECT(static_cast<int>(buffer[7]) == 255);
  }

  SHOULD("Deep-copy on copy construction") {
    Framebuffer original(2, 2);
    original.clear(vec4(1, 0, 0, 1));

    Framebuffer copy(original);
    copy.plot(0, 0, vec4(0, 1, 0, 1));

    // The copy is modified, but the original should be untouched.
    EXPECT(original.getPixel(0, 0) == vec4(1, 0, 0, 1));
    EXPECT(copy.getPixel(0, 0) == vec4(0, 1, 0, 1));
  }
}

}  // namespace gfx1993
