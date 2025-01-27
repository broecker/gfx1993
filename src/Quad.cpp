#include "Quad.h"

namespace gfx1993 {

using glm::vec2;
using glm::vec3;
using glm::vec4;

Quad::Quad(const vec4 &color) {

  const vec3 normal(0, 0, 1);

  Vertex va = Vertex(vec4(-1, 1, 0, 1), normal, color, vec2(0, 1));
  Vertex vb = Vertex(vec4(-1, -1, 0, 1), normal, color, vec2(0, 0));
  Vertex vc = Vertex(vec4(1, -1, 0, 1), normal, color, vec2(1, 0));
  Vertex vd = Vertex(vec4(1, 1, 0, 1), normal, color, vec2(1, 1));

  vertices =  {va, vb, vc, vd};
  indices = {0,1,2, 2,3,0, 0,2,1, 2,0,3};
}

Quad Quad::makeXZQuad(const glm::vec2& size) {
  Quad q(vec4(1.f));

  const vec2 halfSize = size * 0.5f;
  q.vertices[0].position = vec4(-halfSize.x, 0, -halfSize.y, 1.f);
  q.vertices[1].position = vec4(-halfSize.x, 0,  halfSize.y, 1.f);
  q.vertices[2].position = vec4( halfSize.x, 0,  halfSize.y, 1.f);
  q.vertices[3].position = vec4( halfSize.x, 0, -halfSize.y, 1.f);

  for (int i = 0; i < 4; ++i) {
    q.vertices[i].normal = vec3(0,1,0);
  }

  q.indices = {0,2,1, 2,0,3};

  return q;
}

}  // namespace gfx1993