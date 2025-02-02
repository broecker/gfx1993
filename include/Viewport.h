#ifndef VIEWPORT_INCLUDED
#define VIEWPORT_INCLUDED

#include <glm/glm.hpp>

namespace gfx1993 {

class Viewport {
public:
  Viewport(unsigned int x, unsigned int y,
           unsigned int width, unsigned int height);
  Viewport(const glm::ivec2& origin, const glm::ivec2& size);

  bool isInside(const glm::ivec2 &p) const;

  // Transforms the given NDC vertex coordinates in [-1..1] into window
  // coordinates based on this viewport. The viewport is defined with a top-left
  // origin.
  glm::vec3 calculateWindowCoordinates(const glm::vec4 &ndc) const;

  glm::ivec2 origin;
  glm::ivec2 size;
};

} // namespace gfx1993

#endif