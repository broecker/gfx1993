#ifndef SHADER_INCLUDED
#define SHADER_INCLUDED

#include "../common/Vertex.h"
#include "../common/Colour.h"
#include "ShadingGeometry.h"

namespace render 
{

class VertexShader
{
public:
	virtual ~VertexShader();

	virtual VertexOut transformSingle(const Vertex& in) = 0;

	// for STL algorithms
	VertexOut operator () (const Vertex& in)
	{
		return transformSingle( in );
	}
};

class DefaultVertexTransform : public VertexShader
{
public:
	glm::mat4	modelMatrix;
	glm::mat4	viewMatrix;
	glm::mat4	projectionMatrix;

	VertexOut transformSingle(const Vertex& in) override;

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
	Colour shadeSingle(const ShadingGeometry& in) override;

};

class NormalColourShader : public FragmentShader
{
public:
	Colour shadeSingle(const ShadingGeometry& in) override;

};

}

#endif