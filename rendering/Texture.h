//
// Created by mbroecker on 4/19/20.
//
#ifndef GFX1993_TEXTURE_H
#define GFX1993_TEXTURE_H

#include <memory>
#include <string>

#include <glm/glm.hpp>

namespace gfx1993 {
namespace render {

class Texture {
public:
  virtual ~Texture();

  const glm::vec4 &getTexel(const glm::vec2 &texCoords) const;

  const std::string& getId() const { return id; }

  static std::unique_ptr<Texture> makeFlat(unsigned int width, unsigned int height,
                                           const glm::vec4 &fillColor);

  static std::unique_ptr<Texture> makeCheckerboard(unsigned int width,
                                                   unsigned int height,
                                                   unsigned int checkerSize,
                                                   const glm::vec4 &a,
                                                   const glm::vec4 &b);

  static std::unique_ptr<Texture> loadPPM(const std::string &filename);

private:
  unsigned int width, height;
  glm::vec4 *data;

  std::string id;

  explicit Texture(unsigned int width, unsigned int height, const std::string& id);

  inline const glm::vec4 &getTexel(int x, int y) const {
    const size_t idx = x+y * width;
    assert(idx < width*height);
    return data[idx];
  }

  inline void setTexel(unsigned int x, unsigned int y, const glm::vec4 &c) {
    data[x + y * width] = c;
  }
};

} // namespace render
} // namespace gfx1993

#endif // GFX1993_TEXTURE_H
