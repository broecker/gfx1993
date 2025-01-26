#include "Shader.h"

using glm::mat3;
using glm::mat4;
using glm::vec3;
using glm::vec4;

namespace gfx1993 {
namespace render {

VertexOut DefaultVertexTransform::transformSingle(const Vertex &in) {
  mat4 modelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
  // mat3 normalMatrix =
  // glm::inverse(glm::transpose(glm::mat3(modelViewMatrix)));

  VertexOut result;
  result.clipPosition = modelViewProjectionMatrix * in.position;
  result.worldPosition = vec3(modelMatrix * in.position);

  // this assumes that no non-uniform scaling or shearing takes place
  result.worldNormal = mat3(modelMatrix) * in.normal;
  result.color = in.color;
  result.texcoord = in.texcoord;

  return result;
}

Fragment InputColorShader::shadeSingle(const ShadingGeometry &in) {
  return Fragment{in.color};
}

Fragment NormalColorShader::shadeSingle(const ShadingGeometry &in) {
  vec3 c = abs(normalize(in.normal));
  return Fragment{vec4(c, 1.f)};
}

Fragment SingleColorShader::shadeSingle(const ShadingGeometry &in) {
  return Fragment{color};
}

Fragment TextureShader::shadeSingle(const ShadingGeometry& in) {
  return Fragment{texture->getTexel(in.texcoord, mode)};
}

VertexOut SkyboxVertexShader::transformSingle(const Vertex &in) {
  VertexOut out;
  out.clipPosition = vec4(in.position.x, in.position.y, 1.0, 1.0);
  out.texcoord = in.texcoord;
  out.color = normalize(in.position);

  vec4 farPlanePos = inverseViewProjection * vec4(in.position.x, in.position.y, 1.f, 1.f);
  farPlanePos /= farPlanePos.w;

  vec4 cameraPos = inverseViewProjection * vec4(0, 0, 0, 1);
  cameraPos /= cameraPos.w;

  vec3 viewDir = normalize(vec3(farPlanePos) - vec3(cameraPos));
  out.varying[0] = vec4(viewDir, 0);
  out.color = vec4(viewDir, 0);
  return out;
}

Fragment SkyboxFragmentShader::shadeSingle(const render::ShadingGeometry& in) {
    Fragment out;

    vec3 viewDir = in.varying[0];
    vec3 V = normalize(viewDir);

    vec3 color = V;
    if (V.y > 0.0) {
      color = mix(horizon, sky, V.y);
    }
    else {
      color = mix(horizon, ground, -V.y);
    }

    out.color = vec4(color, 1.0);
    out.discard = false;

    return out;
  }

}  // namespace render
}  // namespace gfx1993