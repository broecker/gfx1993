#ifndef RASTERISER_INCLUDED
#define RASTERISER_INCLUDED

#include "Vertex.h"

class Framebuffer;
class Depthbuffer;
class VertexShader;
class FragmentShader;
class Viewport;


struct LinePrimitive;
struct TrianglePrimitive;

class Rasteriser
{
public:
	VertexShader*	vertexShader;
	FragmentShader*	fragmentShader;

	Framebuffer*	framebuffer;
	Depthbuffer* 	depthbuffer;

	Viewport*		viewport;
	
	Rasteriser();

	unsigned int drawPoints(const VertexList& points) const;

	unsigned int drawLines(const VertexList& vertices, const IndexList& indices) const;
	unsigned int drawTriangles(const VertexList& vertices, const IndexList& indices) const;

	void toggleBoundingBoxes();

private:
	void drawLine(const LinePrimitive& line) const;
	void drawTriangle(const TrianglePrimitive& t) const;

	// vertex transform of the input vertices
	void transformVertices(const VertexList& verticesIn, VertexOutList& out) const;

	bool drawBoundingBoxes;

};

#endif