//
// Created by mbroecker on 4/19/20.
//
#ifndef GFX1993_TEXTURE_H
#define GFX1993_TEXTURE_H

#include <limits>
#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace gfx1993 {
namespace render {


template<typename TexelType>
class TextureInterface {
public:
  TextureInterface(unsigned int w, unsigned int h, const std::string& id) : 
    id(id), width(w), height(h), data(w*h, TexelType()) {}

  virtual ~TextureInterface() = default;

  const std::string& getId() const { return id; }

  enum LookupMode {
    CLAMP,
    REPEAT,
  };

  // Returns a texel in the uv coordinate range [0..1]
  const TexelType& getTexel(const glm::vec2& texCoords, LookupMode mode=CLAMP) const {
    glm::vec2 uv;
    if (mode == REPEAT) {
      uv = texCoords - glm::floor(texCoords);
    }

    if (mode == CLAMP) {
      uv = glm::clamp(texCoords, glm::vec2(0), glm::vec2(1));
    }

    // The coordinates are offset by 0.5 to lie in the /center/ of each texel. See
    // Van Verth, Bishop: Essential Mathematics for Games, 2nd Ed, pg 311 
    glm::vec2 tx = uv * glm::vec2(width-1, height-1) - glm::vec2(0.5);

    unsigned int x = ceil(tx.x);
    unsigned int y = ceil(tx.y);

    return getTexel(x, y);
  }

  // Returns a texel im the image coordinate range [0..width/height)
  const TexelType& getTexel(const glm::ivec2& imgCoords, LookupMode mode=CLAMP) const {
    glm::ivec2 coord;
    if (mode == REPEAT) {
      coord = glm::abs(imgCoords) % glm::ivec2(width, height);
    }
    if (mode == CLAMP) {
      coord = glm::clamp(imgCoords, glm::ivec2(0), glm::ivec2(width-1, height-1));
    }

    return getTexel(coord.x, coord.y);
  }

  unsigned int getWidth() const { return width; }
  unsigned int getHeight() const { return height; }

protected:
  std::string               id;
  unsigned int              width, height;
  std::vector<TexelType>    data;

  inline void setTexel(unsigned int x, unsigned int y, const TexelType& c) {
    assert(x < width);
    assert(y < height);
    data[x + y * width] = c;
  }

  inline const TexelType& getTexel(unsigned int x, unsigned int y) const {
    assert(x < width);
    assert(y < height);
    return data[x + y*width];
  }
};

class Texture : public TextureInterface<glm::vec4> {
public:
  static std::unique_ptr<Texture> makeFlat(unsigned int width, unsigned int height,
                                           const glm::vec4 &fillColor);

  static std::unique_ptr<Texture> makeCheckerboard(unsigned int width,
                                                   unsigned int height,
                                                   unsigned int checkerSize,
                                                   const glm::vec4 &a,
                                                   const glm::vec4 &b);

  static std::unique_ptr<Texture> loadPPM(const std::string &filename);

  static std::unique_ptr<Texture> fromVec4s(unsigned int width, 
                                            unsigned int height,
                                            const std::vector<glm::vec4>& data);

  static std::unique_ptr<Texture> perlinNoise(unsigned int width, 
                                              unsigned int height,
                                              const glm::vec2& scale = glm::vec2(1));

private:
  Texture(unsigned int width, unsigned int height, const std::string& id) : 
    TextureInterface<glm::vec4>(width, height, id) {}
};

class HeightMap : public TextureInterface<float> {
public:
  static std::unique_ptr<HeightMap> makeFlat(unsigned int x, unsigned int z);

  static std::unique_ptr<HeightMap> perlinNoise(
    unsigned int width, unsigned int height,
    const glm::vec3& scale = glm::vec3(1),
    const glm::vec3& offset = glm::vec3(0));

  void setHeight(unsigned int x, unsigned int z, float height) {
    setTexel(x, z, height);
    maxHeight = glm::max(maxHeight, height);
    minHeight = glm::min(minHeight, height);
  }

  float getMinHeight() const { return minHeight; }
  float getMaxHeight() const { return maxHeight; }

private:
  HeightMap(unsigned int width, unsigned int height, const std::string& id) : 
    TextureInterface<float>(width, height, id),
    minHeight(std::numeric_limits<float>::max()),
    maxHeight(std::numeric_limits<float>::min()) {}

  float minHeight, maxHeight;
};

} // namespace render
} // namespace gfx1993

#endif // GFX1993_TEXTURE_H
