#include "BoundingVolumes.h"
#include "../base/Pipeline.h"

#include <vector>

namespace gfx1993 {
namespace util {

using glm::vec3;
using glm::vec4;

BoundingSphere fromGeometry(const geometry::Geometry& geo) {
  // Calculate center.
  vec3 center(0.f);
  for (const render::Vertex& v : geo.getVertices()) {
    center += vec3(v.position);
  }
  center /= static_cast<float>(geo.getVertices().size());

  // Calculate extends.
  float radiusSquared = 0.f;
  for (const render::Vertex& v : geo.getVertices()) {
    vec3 delta = vec3(v.position) - center;
    float dist = dot(delta, delta);
    radiusSquared = glm::max(dist, radiusSquared);    
  }

  return BoundingSphere{center, sqrtf(radiusSquared)};
}


}  // namespace util
}  // namespace gfx1993
