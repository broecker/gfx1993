#include <GUnit.h>

#include <limits>

#include <glm/ext.hpp>
#include <glm/gtc/epsilon.hpp>

#include "Texture.h"

namespace gfx1993 {
namespace render {

using glm::vec2;
using glm::vec4;

GTEST("Texture Test") {
  SHOULD("Create flat texture") {
    auto tex = Texture::makeFlat(2, 2, vec4(1,0,1,1));
    
    ASSERT(tex != nullptr);
    EXPECT(tex->getTexel(vec2(0,0)) == vec4(1,0,1,1));
    EXPECT(tex->getTexel(vec2(1,0)) == vec4(1,0,1,1));
    EXPECT(tex->getTexel(vec2(0,1)) == vec4(1,0,1,1));
    EXPECT(tex->getTexel(vec2(1,1)) == vec4(1,0,1,1));
  }

  SHOULD("Create checkerboard") {
    auto tex = Texture::makeCheckerboard(4, 4, 2, 
      vec4(1), vec4(0));

    ASSERT(tex != nullptr);
    EXPECT(tex->getTexel(vec2(0,0)) == vec4(1));
    EXPECT(tex->getTexel(vec2(1,0)) == vec4(0));
    EXPECT(tex->getTexel(vec2(0,1)) == vec4(1));
    EXPECT(tex->getTexel(vec2(1,1)) == vec4(0));
  }

  SHOULD("Use texel centerpoint for coordinates") {
    auto tex = Texture::makeCheckerboard(2, 2, 1, 
      vec4(1), vec4(0));

    ASSERT(tex != nullptr);
    EXPECT(tex->getTexel(vec2(0.25,0)) == vec4(1));
    EXPECT(tex->getTexel(vec2(0.5,0)) == vec4(1));
    std::clog << "tex: " << glm::to_string(tex->getTexel(vec2(0.75, 0))) << std::endl;
    EXPECT(tex->getTexel(vec2(0.75,0)) == vec4(0));
    EXPECT(tex->getTexel(vec2(1,0)) == vec4(0));
  }

  SHOULD("Return nullptr for mismatched texture size") {
    std::vector<vec4> data;

    EXPECT(Texture::fromVec4s(128, 128, data) == nullptr);
  }

  SHOULD("Create raw texture") {
    std::vector<vec4> data = {vec4(1,0,0,1), vec4(1,1,0,1), vec4(0,1,0,1), vec4(0,0,1,1)};

    auto tex = Texture::fromVec4s(2, 2, data);
    
    ASSERT(tex != nullptr);
    EXPECT(tex->getTexel(vec2(0.5, 0)) == vec4(1,0,0,1));
  }

  SHOULD("Clamp texture coordinates") {
    std::vector<vec4> data = {vec4(1,0,0,1), vec4(1,1,0,1), vec4(0,1,0,1), vec4(0,0,1,1)};

    auto tex = Texture::fromVec4s(2, 2, data);

    ASSERT(tex != nullptr);
    EXPECT(tex->getTexel(vec2(0), Texture::CLAMP) == vec4(1,0,0,1));
    EXPECT(tex->getTexel(vec2(-0.5, 0), Texture::CLAMP) == vec4(1,0,0,1));

    EXPECT(tex->getTexel(vec2(1, 0), Texture::CLAMP) == vec4(1,1,0,1));
    EXPECT(tex->getTexel(vec2(5, 0), Texture::CLAMP) == vec4(1,1,0,1));
  }

  SHOULD("Repeat texture coordinates") {
    std::vector<vec4> data = {vec4(1,0,0,1), vec4(1,1,0,1), vec4(0,1,0,1), vec4(0,0,1,1)};

    auto tex = Texture::fromVec4s(2, 2, data);

    ASSERT(tex != nullptr);
    EXPECT(tex->getTexel(vec2(0), Texture::REPEAT) == vec4(1,0,0,1));
    EXPECT(tex->getTexel(vec2(-0.5, 0), Texture::REPEAT) == vec4(0,1,0,1));

    EXPECT(tex->getTexel(vec2(1, 0), Texture::REPEAT) == vec4(1,1,0,1));
    EXPECT(tex->getTexel(vec2(3, 0), Texture::REPEAT) == vec4(1,1,0,1));
  }

  SHOULD("Perlin noise output is repeatable") {
    auto tex = Texture::perlinNoise(32, 32);
    ASSERT(tex != nullptr);

    auto tex2 = Texture::perlinNoise(32, 32);
    ASSERT(tex2 != nullptr);

    EXPECT(tex->getTexel(vec2(0,0)) == tex2->getTexel(vec2(0,0)));
  }
};



}  // namespace render
}  // namespace gfx1993