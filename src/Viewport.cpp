#include "Viewport.h"

namespace gfx1993 {

Viewport::Viewport(unsigned int x, unsigned int y, unsigned int w,
                   unsigned int h)
    : origin(x, y), size(w, h) {}

Viewport::Viewport(const glm::ivec2& origin, const glm::ivec2& size)
  : origin(origin), size(size) {}

glm::vec3 Viewport::calculateWindowCoordinates(const glm::vec3 &ndc) const {
  const float rangeFar = 1.f;
  const float rangeNear = 0.f;

  glm::vec2 pos = glm::vec2(ndc.x, ndc.y) * glm::vec2(size) / 2.f +
                  glm::vec2(origin + size / 2);
  float z =
      ((rangeFar - rangeNear) / 2.f) * ndc.z + (rangeFar + rangeNear) / 2.f;

  return glm::vec3(pos.x, pos.y, z);
}

bool Viewport::isInside(const glm::ivec2 &p) const {
  glm::ivec2 t = p - origin;
  return (t.x >= 0 && t.y >=0 && t.x < size.x && t.y < size.y);
}

}  // namespace gfx1993