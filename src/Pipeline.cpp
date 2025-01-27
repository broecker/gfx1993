#include "Pipeline.h"

#include <glm/glm.hpp>

using glm::vec4;
using glm::mix;

namespace gfx1993 {

VertexOut lerp(const VertexOut &a, const VertexOut &b, float d) {
  VertexOut result;
  result.clipPosition = glm::mix(a.clipPosition, b.clipPosition, d);
  result.worldPosition = glm::mix(a.worldPosition, b.worldPosition, d);
  result.worldNormal = glm::mix(a.worldNormal, b.worldNormal, d);
  result.color = glm::mix(a.color, b.color, d);
  result.texcoord = glm::mix(a.texcoord, b.texcoord, d);
  for (int i = 0; i < GFX1993_SHADER_VARYING_COUNT; ++i) {
    result.varying[i] = glm::mix(a.varying[i], b.varying[i], d);
  }
  return result;
}

ShadingGeometry interpolate(const ShadingGeometry &a,
                            const ShadingGeometry &b, float d) {
  ShadingGeometry result;

  result.position = mix(a.position, b.position, d);
  result.normal = normalize(mix(a.normal, b.normal, d));

  result.color = mix(a.color, b.color, d);
  result.windowCoord = mix(a.windowCoord, b.windowCoord, d);

  result.texcoord = mix(a.texcoord, b.texcoord, d);
  result.depth = mix(a.depth, b.depth, d);

  for (int i = 0; i < GFX1993_SHADER_VARYING_COUNT; ++i) {
    result.varying[i] = mix(a.varying[i], b.varying[i], d);
  }

  return result;
}

ShadingGeometry PointPrimitive::rasterize() const {
  ShadingGeometry result;
  result.position = p.worldPosition;
  result.normal = p.worldNormal;
  result.color = p.color;
  result.texcoord = p.texcoord;
  for (int i = 0; i < GFX1993_SHADER_VARYING_COUNT; ++i) {
    result.varying[i] = p.varying[i];
  }
  return result;
}

ShadingGeometry LinePrimitive::rasterize(float d) const {
  ShadingGeometry result;
  result.position = mix(a.worldPosition, b.worldPosition, d);
  result.normal = normalize(mix(a.worldNormal, b.worldNormal, d));
  result.color = mix(a.color, b.color, d);
  result.texcoord = mix(a.texcoord, b.texcoord, d);
  for (int i = 0; i < GFX1993_SHADER_VARYING_COUNT; ++i) {
    result.varying[i] = mix(a.varying[i], b.varying[i], d);
  }
  return result;
}

// Three-way LERP along barycentric coordinates.
template<typename glm_vec>
static inline glm_vec baryLerp(const glm_vec& a,
                               const glm_vec& b,
                               const glm_vec& c,
                               const glm::vec3& bary) {
  float bsum = bary.x + bary.y + bary.z;
  return (a * bary.x + 
          b * bary.y + 
          c * bary.z) / bsum;
}

ShadingGeometry TrianglePrimitive::rasterize(const glm::vec3 &bary) const {
  ShadingGeometry sgeo;
  sgeo.position = baryLerp(a.worldPosition, b.worldPosition, c.worldPosition, bary);
  sgeo.normal = baryLerp(a.worldNormal, b.worldNormal, c.worldNormal, bary);
  sgeo.color = baryLerp(a.color, b.color, c.color, bary);
  sgeo.texcoord = baryLerp(a.texcoord, b.texcoord, c.texcoord, bary);

  for (int i = 0; i < GFX1993_SHADER_VARYING_COUNT; ++i) {
    sgeo.varying[i] = baryLerp(a.varying[i], b.varying[i], c.varying[i], bary);
  }
  sgeo.surfaceNormal = surfaceNormal;
  return sgeo;
}

}  // namespace gfx1993