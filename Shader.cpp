#include "Shader.h"
#include "glmHelper.h"

VertexShader::~VertexShader()
{
}

Vertex DefaultVertexTransform::transformSingle(const Vertex& in)
{
	glm::mat4 modelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
	glm::mat4 normalMatrix = glm::inverse(glm::transpose(viewMatrix * modelMatrix));

	return Vertex( modelViewProjectionMatrix * in.position,
					in.colour);
}


FragmentShader::~FragmentShader()
{
	
}