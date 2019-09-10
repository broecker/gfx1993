#ifndef RASTERISER_INCLUDED
#define RASTERISER_INCLUDED

#include <memory>

#include "../common/Vertex.h"

namespace render 
{

class Framebuffer;
class Depthbuffer;
class VertexShader;
class FragmentShader;
class Viewport;


struct LinePrimitive;
struct TrianglePrimitive;

class Rasterizer
{
public:	
	std::shared_ptr<VertexShader>	vertexShader;
	std::shared_ptr<FragmentShader>	fragmentShader;

	std::shared_ptr<Framebuffer>	framebuffer;
	std::shared_ptr<Depthbuffer> 	depthbuffer;

	std::shared_ptr<Viewport>		viewport;

	Rasterizer();

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

}

#endif