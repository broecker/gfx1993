#include "Viewport.h"

using glm::ivec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

namespace gfx1993 {

Viewport::Viewport(unsigned int x, unsigned int y, unsigned int w,
                   unsigned int h)
    : origin(x, y), size(w, h) {}

Viewport::Viewport(const ivec2& origin, const ivec2& size)
  : origin(origin), size(size) {}

vec3 Viewport::calculateWindowCoordinates(const vec4 &ndc) const {
  const float vr = origin.x + size.x;
  const float vl = origin.x;
  const float vt = origin.y;
  // This defines the window origin as top-left, with y coordinates increasing
  // to the bottom of the viewport / window.
  const float vb = origin.y + size.y;

  const mat4 viewportTransform(
    vec4((vr-vl) / 2.f, 0.f, 0.f, 0.f),
    vec4(0.f, (vt-vb) / 2.f, 0.f, 0.f),
    vec4(0.f, 0.f, 0.5f, 0.f),
    vec4((vr+vl)/2.f, (vt+vb)/2.f, 0.5f, 1.f));

  return vec3(viewportTransform * ndc);
}

bool Viewport::isInside(const ivec2 &p) const {
  ivec2 t = p - origin;
  return (t.x >= 0 && t.y >=0 && t.x < size.x && t.y < size.y);
}

}  // namespace gfx1993