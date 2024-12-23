//
// Created by mbroecker on 4/19/20.
//
#ifndef GFX1993_TEXTURE_H
#define GFX1993_TEXTURE_H

#include <memory>
#include <string>
#include <vector>

#include <glm/glm.hpp>

namespace gfx1993 {
namespace render {

class Texture {
public:
  virtual ~Texture();

  enum LookupMode {
    CLAMP,
    REPEAT,
  };

  const glm::vec4 &getTexel(const glm::vec2 &texCoords, LookupMode mode=CLAMP) const;

  const std::string& getId() const { return id; }

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
  unsigned int width, height;
  glm::vec4 *data;

  // For debugging purposes.
  std::string id;

  explicit Texture(unsigned int width, unsigned int height, const std::string& id);

  inline const glm::vec4 &getTexel(unsigned int x, unsigned int y) const {
    assert(x < width);
    assert(y < height);
    return data[x + y*width];
  }

  inline void setTexel(unsigned int x, unsigned int y, const glm::vec4 &c) {
    assert(x < width);
    assert(y < height);
    data[x + y * width] = c;
  }
};

} // namespace render
} // namespace gfx1993

#endif // GFX1993_TEXTURE_H
