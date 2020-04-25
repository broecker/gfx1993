#include "Quad.h"

namespace geometry {
Quad::Quad(const glm::vec4 &color) {
  using glm::vec2;
  using glm::vec4;
  using render::Vertex;

  const glm::vec3 normal(0, 0, 1);

  Vertex va = Vertex(vec4(-1, 1, 0, 1), normal, color, vec2(0, 1));
  Vertex vb = Vertex(vec4(-1, -1, 0, 1), normal, color, vec2(0, 0));
  Vertex vc = Vertex(vec4(1, -1, 0, 1), normal, color, vec2(1, 0));
  Vertex vd = Vertex(vec4(1, 1, 0, 1), normal, color, vec2(1, 1));

  vertices.push_back(va);
  vertices.push_back(vb);
  vertices.push_back(vc);
  vertices.push_back(vd);

  indices.push_back(0);
  indices.push_back(1);
  indices.push_back(2);
  indices.push_back(2);
  indices.push_back(3);
  indices.push_back(0);

  // And back faces (although normal is wrong ... )
  indices.push_back(0);
  indices.push_back(2);
  indices.push_back(1);
  indices.push_back(2);
  indices.push_back(0);
  indices.push_back(3);
}
}