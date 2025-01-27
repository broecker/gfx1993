#include <GUnit.h>

#include <limits>

#include <glm/ext.hpp>
#include <glm/gtc/epsilon.hpp>

#include "Texture.h"

namespace gfx1993 {

using glm::ivec2;
using glm::vec2;
using glm::vec4;

namespace {

// Define a couple of colors.
constexpr vec4 R(1,0,0,1);
constexpr vec4 Y(1,1,0,1);
constexpr vec4 G(0,1,0,1);
constexpr vec4 B(0,0,1,1);
constexpr vec4 hotPink(1,0,1,1);

// Creates a 2x2 texture for testing:
//  +---+---+
//  | R | Y |
//  +---+---+
//  | G | B |
//  +---+---+
static std::unique_ptr<Texture> makeTestTexture() {
  auto tex = Texture::fromVec4s(2, 2, {R, Y, G, B});
  assert(tex != nullptr);
  return tex;
}

}  // namespace

GTEST("Texture Test") {
  SHOULD("Create flat texture 1") {
    auto tex = Texture::makeFlat(1, 1, hotPink);
    
    ASSERT(tex != nullptr);
    EXPECT(tex->getTexel(vec2(0,0)) == hotPink);
    EXPECT(tex->getTexel(vec2(1,0)) == hotPink);
    EXPECT(tex->getTexel(vec2(0,1)) == hotPink);
    EXPECT(tex->getTexel(vec2(1,1)) == hotPink);
    EXPECT(tex->getTexel(vec2(0.5)) == hotPink);
  }

  SHOULD("Create flat texture 2") {
    auto tex = Texture::makeFlat(2, 2, hotPink);
    
    ASSERT(tex != nullptr);
    EXPECT(tex->getTexel(vec2(0,0)) == hotPink);
    EXPECT(tex->getTexel(vec2(1,0)) == hotPink);
    EXPECT(tex->getTexel(vec2(0,1)) == hotPink);
    EXPECT(tex->getTexel(vec2(1,1)) == hotPink);
    EXPECT(tex->getTexel(vec2(0.5)) == hotPink);
  }

  SHOULD("Create checkerboard") {
    // +---+---+
    // | 1 | 0 |
    // +---+---+
    // | 0 | 1 |
    // +---+---+
    auto tex = Texture::makeCheckerboard(4, 4, 2, 
      vec4(1), vec4(0));

    ASSERT(tex != nullptr);
    EXPECT(tex->getTexel(vec2(0,0)) == vec4(1));
    EXPECT(tex->getTexel(vec2(1,0)) == vec4(0));
    EXPECT(tex->getTexel(vec2(0,1)) == vec4(1));
    EXPECT(tex->getTexel(vec2(1,1)) == vec4(0));
    EXPECT(tex->getTexel(vec2(0.5)) == vec4(0));
  }

  SHOULD("Use texel centerpoint for coordinates") {
    auto tex = Texture::makeCheckerboard(2, 2, 1, 
      vec4(1), vec4(0));

    ASSERT(tex != nullptr);
    EXPECT(tex->getTexel(vec2(0.25,0)) == vec4(1));
    EXPECT(tex->getTexel(vec2(0.5,0)) == vec4(1));
    EXPECT(tex->getTexel(vec2(0.75,0)) == vec4(0));
    EXPECT(tex->getTexel(vec2(1,0)) == vec4(0));
  }

  SHOULD("Return nullptr for mismatched texture size") {
    std::vector<vec4> data;

    EXPECT(Texture::fromVec4s(32, 32, data) == nullptr);
  }

  SHOULD("Create raw texture") {
    auto tex = makeTestTexture();
    EXPECT(tex->getTexel(vec2(0.5, 0)) == vec4(1,0,0,1));
  }

  SHOULD("Clamp texture coordinates") {
    auto tex = makeTestTexture();
    EXPECT(tex->getTexel(vec2(0), Texture::CLAMP) == R);
    EXPECT(tex->getTexel(vec2(-0.5, 0), Texture::CLAMP) == R);
    EXPECT(tex->getTexel(vec2(1, 0), Texture::CLAMP) == Y);
    EXPECT(tex->getTexel(vec2(5, 0), Texture::CLAMP) == Y);
  }

  SHOULD("Clamp image coordinates") {
    auto tex = makeTestTexture();
    EXPECT(tex->getTexel(ivec2(0), Texture::CLAMP) == R);
    EXPECT(tex->getTexel(ivec2(-1, 0), Texture::CLAMP) == R);
    EXPECT(tex->getTexel(ivec2(1, 0), Texture::CLAMP) == Y);
    EXPECT(tex->getTexel(ivec2(5, 0), Texture::CLAMP) == Y);
  }

  SHOULD("Repeat texture coordinates horizontally") {
    auto tex = makeTestTexture();
    // (0,0) (1,0)
    //   R     Y
    //   G     B
    // (0,1)  (1,1)

    EXPECT(tex->getTexel(vec2(0), Texture::REPEAT) == R);
    EXPECT(tex->getTexel(vec2(0.9,0), Texture::REPEAT) == Y);
    EXPECT(tex->getTexel(vec2(1,0), Texture::REPEAT) == R);
    EXPECT(tex->getTexel(vec2(1.6,0), Texture::REPEAT) == Y);
  }

  SHOULD("Repeat texture coordinates vertically") {
    auto tex = makeTestTexture();
    // (0,0) (1,0)
    //   R     Y
    //   G     B
    // (0,1)  (1,1)

    EXPECT(tex->getTexel(vec2(0), Texture::REPEAT) == R);
    EXPECT(tex->getTexel(vec2(0,0.6), Texture::REPEAT) == G);
    EXPECT(tex->getTexel(vec2(0,1), Texture::REPEAT) == R);
    EXPECT(tex->getTexel(vec2(0,0.6), Texture::REPEAT) == G);
  }

  SHOULD("Repeat image coordinates horizontally") {
    auto tex = makeTestTexture();
    EXPECT(tex->getTexel(ivec2(0), Texture::REPEAT) == R);
    EXPECT(tex->getTexel(ivec2(1,0), Texture::REPEAT) == Y);
    EXPECT(tex->getTexel(ivec2(2,0), Texture::REPEAT) == R);
    EXPECT(tex->getTexel(ivec2(3,0), Texture::REPEAT) == Y);
  }

  SHOULD("Repeat image coordinates vertically") {
    auto tex = makeTestTexture();
    // (0,0) (1,0)
    //   R     Y
    //   G     B
    // (0,1)  (1,1)

    EXPECT(tex->getTexel(ivec2(0), Texture::REPEAT) == R);
    EXPECT(tex->getTexel(ivec2(0,1), Texture::REPEAT) == G);
    EXPECT(tex->getTexel(ivec2(0,2), Texture::REPEAT) == R);
    EXPECT(tex->getTexel(ivec2(0,3), Texture::REPEAT) == G);
  }


  SHOULD("Perlin noise output is repeatable") {
    auto tex = Texture::perlinNoise(32, 32);
    ASSERT(tex != nullptr);

    auto tex2 = Texture::perlinNoise(32, 32);
    ASSERT(tex2 != nullptr);

    EXPECT(tex->getTexel(vec2(0,0)) == tex2->getTexel(vec2(0,0)));
  }
};

}  // namespace gfx1993