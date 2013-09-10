#ifndef RENDERER_INCLUDED
#define RENDERER_INCLUDED

#include "Vertex.h"

class Framebuffer;
class Depthbuffer;
class VertexShader;

class Renderer
{
public:
	VertexShader*	vertexShader;

	Framebuffer*	framebuffer;
	Depthbuffer* 	depthbuffer;

	unsigned int drawLines(const VertexList& vertices, const IndexList& indices) const;

private:
	void bresenhamLine();

};

#endif