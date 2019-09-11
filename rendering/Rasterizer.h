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
    // The output where the renderer writes to.
    struct RenderOutput
    {
        // The color buffer.
        std::shared_ptr<Framebuffer>	framebuffer;
        // The depth buffer.
        std::shared_ptr<Depthbuffer> 	depthbuffer;
        // The viewport defining the renderable area.
        std::shared_ptr<Viewport>		viewport;
    };

    // Holds the vertex and fragment shader used for rendering.
	struct ShaderConfiguration
    {
        std::shared_ptr<VertexShader>	vertexShader;
        std::shared_ptr<FragmentShader>	fragmentShader;

        bool isValid() const;
    };

	Rasterizer();
	virtual ~Rasterizer() = default;

	// Draws all the points given.
	unsigned int drawPoints(const VertexList &points) const;
	// Draws the vertices as lines. Every two indices define the endpoints of a line.
	unsigned int drawLines(const VertexList &vertices, const IndexList &indices) const;
	// TODO: Add line loop/line strip.
	// Draws the vertices as lines. Every three indices are treated as the three corners of a triangle. Triangles are
	// defined counter-clockwise.
	unsigned int drawTriangles(const VertexList &vertices, const IndexList &indices) const;

	// Toggles debug display of screen-space bounding boxes (that are used for rasterization).
	void toggleBoundingBoxes();

	// Sets the current render target.
	inline void setRenderOutput(const RenderOutput& output) { this->output = output; }
	// Sets the current vertex and fragment shader.
	inline void setShaders(const ShaderConfiguration& shaders) { this->shaders = shaders; }

private:
	void drawLine(const LinePrimitive &line) const;
	void drawTriangle(const TrianglePrimitive &t) const;

	// vertex transform of the input vertices
	void transformVertices(const VertexList& verticesIn, VertexOutList& out, std::shared_ptr<VertexShader> vertexShader) const;

	RenderOutput            output;
	ShaderConfiguration     shaders;

	bool drawBoundingBoxes;
};

}

#endif