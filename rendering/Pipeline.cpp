#include "Pipeline.h"

#include <glm/glm.hpp>

using glm::vec4;

namespace render {
VertexOut &&lerp(const VertexOut &a, const VertexOut &b, float d) {
  VertexOut result;
  result.clipPosition = glm::mix(a.clipPosition, b.clipPosition, d);
  result.worldPosition = glm::mix(a.worldPosition, b.worldPosition, d);
  result.worldNormal = glm::mix(a.worldNormal, b.worldNormal, d);
  result.color = glm::mix(a.color, b.color, d);
  result.texcoord = glm::mix(a.texcoord, b.texcoord, d);
  return std::move(result);
}

ShadingGeometry &&interpolate(const ShadingGeometry &a,
                              const ShadingGeometry &b, float d) {
  ShadingGeometry result;

  result.position = mix(a.position, b.position, d);
  result.normal = normalize(mix(a.normal, b.normal, d));

  result.color = mix(a.color, b.color, d);
  result.windowCoord = mix(a.windowCoord, b.windowCoord, d);

  return std::move(result);
}

ShadingGeometry &&PointPrimitive::rasterize() const {
  ShadingGeometry result;
  result.position = p.worldPosition;
  result.normal = p.worldNormal;
  result.color = p.color;
  result.texcoord = p.texcoord;
  return std::move(result);
}

ShadingGeometry &&LinePrimitive::rasterize(float d) const {
  ShadingGeometry result;
  result.position = mix(a.worldPosition, b.worldPosition, d);
  result.normal = normalize(mix(a.worldNormal, b.worldNormal, d));
  result.color = mix(a.color, b.color, d);
  result.texcoord = mix(a.texcoord, b.texcoord, d);
  return std::move(result);
}

ShadingGeometry &&TrianglePrimitive::rasterize(const glm::vec3 &bary) const {
  float bsum = bary.x + bary.y + bary.z;

  ShadingGeometry sgeo;
  sgeo.position = a.worldPosition * bary.x + b.worldPosition * bary.y +
                  c.worldPosition * bary.z / bsum;
  sgeo.normal = a.worldNormal * bary.x + b.worldNormal * bary.y +
                c.worldNormal * bary.z / bsum;
  sgeo.color = a.color * bary.x + b.color * bary.y + c.color * bary.z / bsum;
  sgeo.texcoord =
      a.texcoord * bary.x + b.texcoord * bary.y + c.texcoord * bary.z / bsum;

  return std::move(sgeo);
}

} // namespace render
