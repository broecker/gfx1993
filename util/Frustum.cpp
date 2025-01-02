#include "Frustum.h"

#include <array>
#include <vector>

#include "Camera.h"

namespace gfx1993 {
namespace util {

using namespace glm;

// These are the clip-space coordinates of the frustum.
// 
// near:          far:
// 0   3         4   7
// +---+   ....  +---+ 
// |   |         |   |
// +---+   ....  +---+
// 1   2         5   6
//
//         +y   
//          ^
//          | 
//          +---> +x
//         /
//        +z

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

const static std::vector<unsigned int> frustumIndices = {
  0,1,1,2,2,3,3,0, 4,5,5,6,6,7,7,4, 0,4,1,5,2,6,3,7,

  // Debug output for normals:
  8,9,    // near
  10,11,  // far
  12,13,  // left
  14,15,  // right
  16,17,  // top
  18,19   // bottom
};

inline vec4 getCenter(const render::Vertex& a,
                      const render::Vertex& b,
                      const render::Vertex& c,
                      const render::Vertex& d) {
  return (a.position + b.position + c.position + d.position) / 4.f;
}

Frustum::Frustum(const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix) : 
  projectionMatrix(projectionMatrix), viewMatrix(viewMatrix) {
  for (int i = 0; i < 8; ++i) {
    render::Vertex v;
    v.position = frustumVertices[i];
    v.normal = vec4(0);
    v.color = vec4(1,0,1,1);
    v.texcoord = vec2(0);
    vertices.push_back(v);
  }

  // Also add 6*2 = 12 vertices to draw one normal per plane.
  for (int i = 0; i < 12; ++i) {
    render::Vertex v;
    v.position = vec4(0);
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

  // Calculate all plane equations here. The normals are point inwards into the
  // frustum -- i.e. the positive halfspace of each plane is inside the visible
  // volume.
  planes[NEAR].set(vec3(vertices[0].position),
                   vec3(vertices[2].position),
                     vec3(vertices[1].position));
  planes[FAR].set(vec3(vertices[4].position),
                  vec3(vertices[5].position),
                    vec3(vertices[6].position));
  planes[LEFT].set(vec3(vertices[0].position),
                   vec3(vertices[1].position),
                     vec3(vertices[4].position));
  planes[RIGHT].set(vec3(vertices[2].position),
                   vec3(vertices[3].position),
                     vec3(vertices[7].position));
  planes[TOP].set(vec3(vertices[3].position),
                  vec3(vertices[0].position),
                    vec3(vertices[4].position));
  planes[BOTTOM].set(vec3(vertices[1].position),
                   vec3(vertices[2].position),
                     vec3(vertices[5].position));

  // Debug normals:

  // Near plane.
  vertices[8].position = getCenter(vertices[0], vertices[1], vertices[2], vertices[3]);
  vertices[9].position = vertices[8].position + vec4(planes[NEAR].normal, 0);
  // Far plane.
  vertices[10].position = getCenter(vertices[4], vertices[5], vertices[6], vertices[7]);
  vertices[11].position = vertices[10].position + vec4(planes[FAR].normal, 0);
  // Left
  vertices[12].position = getCenter(vertices[0], vertices[1], vertices[4], vertices[5]);
  vertices[13].position = vertices[12].position + vec4(planes[LEFT].normal, 0);
  // Right
  vertices[14].position = getCenter(vertices[2], vertices[3], vertices[6], vertices[7]);
  vertices[15].position = vertices[14].position + vec4(planes[RIGHT].normal, 0);
  // Top
  vertices[16].position = getCenter(vertices[0], vertices[3], vertices[4], vertices[7]);
  vertices[17].position = vertices[16].position + vec4(planes[TOP].normal, 0);
  // Bottom
  vertices[18].position = getCenter(vertices[1], vertices[2], vertices[5], vertices[6]);
  vertices[19].position = vertices[18].position + vec4(planes[BOTTOM].normal, 0);  
}

const IndexList& Frustum::getIndices() const {
  return frustumIndices;
}

const VertexList& Frustum::getVertices() const {
  return vertices;
}

// See Akenine-Moeller; Realtime-Rendering 2nd Ed, A5.2
void Frustum::Plane::set(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
  normal = normalize(cross(b-a, c-a));
  d = dot(-normal, a);
}

bool Frustum::isInside(const glm::vec3& pt) const {
  for (int i = 0; i < PLANES_COUNT; ++i) {
    if (planes[i].distance(pt) < 0) {
      return false;
    }
  }
  return true;
}

Frustum::IntersectionResult Frustum::testIntersection(const BoundingSphere& sphere) const {
  for (int i = 0; i < PLANES_COUNT; ++i) {
    const float d = planes[i].distance(sphere.center);
    if (d > sphere.radius) {
      return OUTSIDE;
    } else if (d < sphere.radius) {
      return INTERSECTING;
    }
  }
  return INSIDE;
}

Frustum::IntersectionResult Frustum::testIntersection(const AABB& box) const {
  // Algorithm from Realtime-Rendering, 2nd ed. 16.10.1
  const vec3 c = (box.max + box.min) / 2.f;
  const vec3 h = (box.max - box.min) / 2.f;

  IntersectionResult result = INSIDE;
  for (int i = 0; i < PLANES_COUNT; ++i) {
    const Plane& p = planes[i];

    float e = h.x*abs(p.normal.x) + h.y*abs(p.normal.y) + h.z*abs(p.normal.z);
    float s = glm::dot(c, p.normal) + p.d;

    // Our frustum is defined with the positive half-space pointing /out/. Hence
    // the opposite return value as in the book.
    if (s - e > 0.f) {
      result = INSIDE;
    } else if (s + e < 0.f) {
      return OUTSIDE;
    } else {
      result = INTERSECTING;
    }
  }
  return result;
}

}  // namespace util
}  // namespace gfx1993
