#ifndef RASTERISER_INCLUDED
#define RASTERISER_INCLUDED

#include <memory>

#include "Pipeline.h"
#include "Clipper.h"

namespace render
{
    // Forward declarations.
    class Framebuffer;
    class Depthbuffer;
    class VertexShader;
    class FragmentShader;
    class Viewport;

    // Main class that does the heavy lifting in putting fragments into the framebuffer.
    class Rasterizer
    {
    public:
        // The output where the renderer writes to.
        struct RenderOutput
        {
            // The color buffer.
            std::shared_ptr<Framebuffer> framebuffer;
            // The depth buffer.
            std::shared_ptr<Depthbuffer> depthbuffer;
            // The viewport defining the renderable area.
            std::shared_ptr<Viewport> viewport;
        };

        // Holds the vertex and fragment shader used for rendering.
        struct ShaderConfiguration
        {
            std::shared_ptr<VertexShader> vertexShader;
            std::shared_ptr<FragmentShader> fragmentShader;

            bool isValid() const;
        };

        Rasterizer();

        virtual ~Rasterizer() = default;

        // Draws the vertices as points. Only the indexed vertices are drawn.
        unsigned int drawPoints(const VertexList& vertices, const IndexList& indices) const;

        // Draws the vertices as lines. Every two indices define the endpoints of a line.
        unsigned int drawLines(const VertexList &vertices, const IndexList &indices) const;

        // Draws a continuous line strip; each index is considered to be the either the midpoint between two lines, or
        // the endpoint in case of the first and last index. To draw a closed loop, make sure that the first and last
        // index are the same; i.e. [0, 1, 2, 3, 0]
        unsigned int drawLineStrip(const VertexList& vertices, const IndexList& indices) const;

        // Draws the vertices as lines. Every three indices are treated as the three corners of a triangle. Triangles are
        // defined counter-clockwise.
        unsigned int drawTriangles(const VertexList &vertices, const IndexList &indices) const;

        // Toggles debug display of screen-space bounding boxes (that are used for rasterization).
        void toggleBoundingBoxes();

        // Sets the current render target.
        inline void setRenderOutput(const RenderOutput &output) { this->output = output; }

        // Sets the current vertex and fragment shader.
        inline void setShaders(const ShaderConfiguration &shaders) { this->shaders = shaders; }

    private:
        // Draws a line after it was clipped to the Viewport.
        void drawLine(const LinePrimitive &line) const;
        // Draws a triangle that was clipped to the viewport.
        void drawTriangle(const TrianglePrimitive &t) const;

        // Vertex transform of the input vertices
        VertexOutList transformVertices(const VertexList &verticesIn, std::shared_ptr<VertexShader> vertexShader) const;

        RenderOutput output;
        ShaderConfiguration shaders;

        std::vector<glm::vec4> clipPlanes;

        Clipper clipper;

        bool drawBoundingBoxes;
    };

}

#endif