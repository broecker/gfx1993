#ifndef SHADER_INCLUDED
#define SHADER_INCLUDED

#include "Pipeline.h"

namespace render 
{

class VertexShader
{
public:
	virtual ~VertexShader() = default;

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
	virtual ~FragmentShader() = default;
	
	virtual Fragment&& shadeSingle(const ShadingGeometry& in) = 0;

};

class InputColourShader : public FragmentShader
{
public:
    Fragment&& shadeSingle(const ShadingGeometry& in) override;

};

class NormalColourShader : public FragmentShader
{
public:
    Fragment&& shadeSingle(const ShadingGeometry& in) override;

};

}

#endif