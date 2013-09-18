#include "Shader.h"
#include "glmHelper.h"

using namespace glm;

VertexShader::~VertexShader()
{
}

VertexOut DefaultVertexTransform::transformSingle(const Vertex& in)
{
	mat4 modelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
	mat3 normalMatrix = glm::mat3(glm::inverse(glm::transpose(viewMatrix * modelMatrix)));

	VertexOut result;
	result.clipPosition = modelViewProjectionMatrix * in.position;
	result.worldPosition = vec3(modelMatrix * in.position);
	result.worldNormal = normalMatrix * in.normal;
	result.colour = in.colour;
	result.texcoord = in.texcoord;

	return result;
}


FragmentShader::~FragmentShader()
{
	
}

Colour InputColourShader::shadeSingle(const ShadingGeometry& in)
{
	return in.colour;
}

Colour NormalColourShader::shadeSingle(const ShadingGeometry& in)
{
	vec3 c = abs(in.normal);
	return Colour( c.x, c.y, c.z, 1.f );
}