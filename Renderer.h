#ifndef RENDERER_INCLUDED
#define RENDERER_INCLUDED

#include "Vertex.h"

class Framebuffer;
class Depthbuffer;
class VertexShader;
class FragmentShader;
class Viewport;

class Renderer
{
public:
	Renderer();

	VertexShader*	vertexShader;
	FragmentShader*	fragmentShader;

	Framebuffer*	framebuffer;
	Depthbuffer* 	depthbuffer;

	Viewport*		viewport;
	
	unsigned int drawPoints(const VertexList& points) const;

	unsigned int drawLines(const VertexList& vertices, const IndexList& indices) const;

private:
	void bresenhamLine(glm::ivec2 a, glm::ivec2 b, glm::vec4 colour);

};

#endif