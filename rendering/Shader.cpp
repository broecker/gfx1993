#include "Shader.h"

using glm::mat3;
using glm::mat4;
using glm::vec3;
using glm::vec4;

namespace render {

VertexOut &&DefaultVertexTransform::transformSingle(const Vertex &in) {
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

  return std::move(result);
}

Fragment &&InputColorShader::shadeSingle(const ShadingGeometry &in) {
  return std::move(Fragment{in.color});
}

Fragment &&NormalColorShader::shadeSingle(const ShadingGeometry &in) {
  vec3 c = abs(normalize(in.normal));
  return std::move(Fragment{vec4(c, 1.f)});
}

Fragment &&SingleColorShader::shadeSingle(const ShadingGeometry &in) {
  return std::move(Fragment{color});
}

}