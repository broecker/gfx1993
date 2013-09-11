#ifndef SHADER_INCLUDED
#define SHADER_INCLUDED

#include "Vertex.h"

class VertexShader
{
public:
	virtual ~VertexShader();

	virtual Vertex transformSingle(const Vertex& in) = 0;
};

class DefaultVertexTransform : public VertexShader
{
public:
	glm::mat4	modelMatrix;
	glm::mat4	viewMatrix;
	glm::mat4	projectionMatrix;


	Vertex transformSingle(const Vertex& in);
};

class FragmentShader
{

};




#endif