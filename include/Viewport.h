#ifndef VIEWPORT_INCLUDED
#define VIEWPORT_INCLUDED

#include <glm/glm.hpp>

namespace gfx1993 {
namespace render {

class Viewport {
public:
  Viewport(unsigned int x, unsigned int y,
           unsigned int width, unsigned int height);
  Viewport(const glm::ivec2& origin, const glm::ivec2& size);

  bool isInside(const glm::ivec2 &p) const;

  glm::vec3 calculateWindowCoordinates(const glm::vec3 &ndc) const;

  glm::ivec2 origin;
  glm::ivec2 size;
};

} // namespace render
} // namespace gfx1993

#endif