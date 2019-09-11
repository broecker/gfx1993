#include "Shader.h"

using namespace glm;

namespace render 
{

VertexOut DefaultVertexTransform::transformSingle(const Vertex& in)
{
	mat4 modelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
	//mat3 normalMatrix = glm::inverse(glm::transpose(glm::mat3(modelViewMatrix)));

	VertexOut result;
	result.clipPosition = modelViewProjectionMatrix * in.position;
	result.worldPosition = vec3(modelMatrix * in.position);
	
	// this assumes that no non-uniform scaling or shearing takes place
	result.worldNormal = mat3(modelMatrix) * in.normal;
	result.colour = in.colour;
	result.texcoord = in.texcoord;

	return result;
}

Color InputColourShader::shadeSingle(const ShadingGeometry& in)
{
	return in.colour;
}

Color NormalColourShader::shadeSingle(const ShadingGeometry& in)
{
	vec3 c = abs(normalize(in.normal));
	return Color(c.x, c.y, c.z, 1.f );
}

}