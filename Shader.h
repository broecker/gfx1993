#ifndef SHADER_INCLUDED
#define SHADER_INCLUDED

#include "Vertex.h"

class VertexShader
{
public:
	virtual ~VertexShader();

	virtual Vertex processSingle(const Vertex& in) = 0;
};

class DefaultVertexTransform : public VertexShader
{
public:
	glm::mat4	modelViewMatrix;
	glm::mat4	projectionMatrix;


	Vertex processSingle(const Vertex& in);
};

class FragmentShader
{

};




#endif