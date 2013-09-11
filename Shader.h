#ifndef SHADER_INCLUDED
#define SHADER_INCLUDED

#include "Vertex.h"
#include "Colour.h"
#include "ShadingGeometry.h"

class VertexShader
{
public:
	virtual ~VertexShader();

	virtual Vertex transformSingle(const Vertex& in) = 0;
	virtual glm::vec3 calculateWorldPosition(const Vertex& in) = 0;
};

class DefaultVertexTransform : public VertexShader
{
public:
	glm::mat4	modelMatrix;
	glm::mat4	viewMatrix;
	glm::mat4	projectionMatrix;


	Vertex transformSingle(const Vertex& in);
	glm::vec3 calculateWorldPosition(const Vertex& in);

};

class FragmentShader
{
public:
	virtual ~FragmentShader();
	
	virtual Colour shadeSingle(const ShadingGeometry& in) = 0;

};

class InputColourShader : public FragmentShader
{
public:
	Colour shadeSingle(const ShadingGeometry& in);

};


#endif