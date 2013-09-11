#include "Shader.h"
#include "glmHelper.h"

VertexShader::~VertexShader()
{
}

Vertex DefaultVertexTransform::transformSingle(const Vertex& in)
{
	glm::mat4 modelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
	glm::mat3 normalMatrix = glm::mat3(glm::inverse(glm::transpose(viewMatrix * modelMatrix)));

	return Vertex( 	modelViewProjectionMatrix * in.position,
					normalMatrix * in.normal,
					in.colour);
}

glm::vec3 DefaultVertexTransform::calculateWorldPosition(const Vertex& in)
{
	return v3( modelMatrix * in.position );
}


FragmentShader::~FragmentShader()
{
	
}

Colour InputColourShader::shadeSingle(const ShadingGeometry& in)
{
	return in.colour;
}