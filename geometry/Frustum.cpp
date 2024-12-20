#include "Frustum.h"

#include <array>

#include "../base/Pipeline.h"
#include "../util/Camera.h"

namespace gfx1993 {
namespace geometry {

using namespace glm;

// These are the clip-space coordinates of the frustum.
const static std::array<vec4, 8> frustumVertices = {
  // Near plane.
  vec4(-1,1,-1,1),
  vec4(-1,-1,-1,1),
  vec4(1,-1,-1,1),
  vec4(1,1,-1,1),
  // Far plane.
  vec4(-1,1,1,1),
  vec4(-1,-1,1,1),
  vec4(1,-1,1,1),
  vec4(1,1,1,1),
};

Frustum::Frustum(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix) : 
  projectionMatrix(projectionMatrix), viewMatrix(viewMatrix) {
  indices = {0,1,1,2,2,3,3,0, 4,5,5,6,6,7,7,4, 0,4,1,5,2,6,3,7};
  for (int i = 0; i < 8; ++i) {
    render::Vertex v;
    v.position = frustumVertices[i];
    v.normal = vec4(0);
    v.color = vec4(1,0,1,1);
    v.texcoord = vec2(0);
    vertices.push_back(v);
  }

  update(viewMatrix);
}

void Frustum::update(const glm::mat4& view) {
  viewMatrix = view;

  glm::mat4 ivp = glm::inverse(projectionMatrix* viewMatrix);
  for (int i = 0; i < 8; ++i) {
    vec4 w = ivp * frustumVertices[i];
    w = w / w.w;
    vertices[i].position = w;
  }
}



}  // namespace geometry
}  // namespace gfx1993
