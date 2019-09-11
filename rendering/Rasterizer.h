#ifndef RASTERISER_INCLUDED
#define RASTERISER_INCLUDED

#include <memory>

#include "Pipeline.h"

namespace render
{

class Framebuffer;
class Depthbuffer;
class VertexShader;
class FragmentShader;
class Viewport;

class Rasterizer
{
public:	
	struct RenderConfiguration
    {
        std::shared_ptr<VertexShader>	vertexShader;
        std::shared_ptr<FragmentShader>	fragmentShader;

        std::shared_ptr<Framebuffer>	framebuffer;
        std::shared_ptr<Depthbuffer> 	depthbuffer;

        std::shared_ptr<Viewport>		viewport;

        bool isValid() const;
    };

	Rasterizer();
	virtual ~Rasterizer() = default;

	unsigned int drawPoints(const VertexList& points, const RenderConfiguration& renderConfiguration) const;
	unsigned int drawLines(const VertexList& vertices, const IndexList& indices, const RenderConfiguration& renderConfig) const;
	unsigned int drawTriangles(const VertexList& vertices, const IndexList& indices, const RenderConfiguration& renderConfiguration) const;

	void toggleBoundingBoxes();

private:
	void drawLine(const LinePrimitive &line, const RenderConfiguration& renderConfig) const;
	void drawTriangle(const TrianglePrimitive &t, const RenderConfiguration& renderConfiguration) const;

	// vertex transform of the input vertices
	void transformVertices(const VertexList& verticesIn, VertexOutList& out, std::shared_ptr<VertexShader> vertexShader) const;

	bool drawBoundingBoxes;
};

}

#endif