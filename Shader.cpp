#include "Shader.h"

VertexShader::~VertexShader()
{
}

Vertex DefaultVertexTransform::processSingle(const Vertex& in)
{
	glm::mat4 modelViewProjectionMatrix = projectionMatrix * modelViewMatrix;
	glm::mat4 normalMatrix = glm::inverse(glm::transpose(modelViewMatrix));

	return Vertex( modelViewProjectionMatrix * in.position,
					in.colour);
}
