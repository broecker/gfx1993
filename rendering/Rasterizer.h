#ifndef RASTERISER_INCLUDED
#define RASTERISER_INCLUDED

#include <memory>

#include "Clipper.h"
#include "Pipeline.h"
#include "RenderConfig.h"

namespace render {
// Main class that does the heavy lifting in putting fragments into the
// framebuffer.
class Rasterizer {
public:
  virtual ~Rasterizer() = default;

  // Draws the vertices as points. Only the indexed vertices are drawn.
  unsigned int drawPoints(const RenderConfig &renderConfig,
                          const VertexList &vertices,
                          const IndexList &indices) const;

  // Draws the vertices as lines. Every two indices define the endpoints of a
  // line.
  unsigned int drawLines(const RenderConfig &renderConfig,
                         const VertexList &vertices,
                         const IndexList &indices) const;

  // Draws a continuous line strip; each index is considered to be the either
  // the midpoint between two lines, or the endpoint in case of the first and
  // last index. To draw a closed loop, make sure that the first and last index
  // are the same; i.e. [0, 1, 2, 3, 0]
  unsigned int drawLineStrip(const RenderConfig &renderConfig,
                             const VertexList &vertices,
                             const IndexList &indices) const;

  // Draws the vertices as lines. Every three indices are treated as the three
  // corners of a triangle. Triangles are defined counter-clockwise.
  unsigned int drawTriangles(const RenderConfig &renderConfig,
                             const VertexList &vertices,
                             const IndexList &indices) const;

private:
  // Draws a line after it was clipped to the Viewport.
  void drawLine(const RenderConfig &renderConfig,
                const LinePrimitive &line) const;

  // Draws a triangle that was clipped to the viewport.
  void drawTriangle(const RenderConfig &renderConfig,
                    const TrianglePrimitive &t) const;

  // Vertex transform of the input vertices
  VertexOutList
  transformVertices(const VertexList &verticesIn,
                    std::shared_ptr<VertexShader> vertexShader) const;

  Clipper clipper;
};

} // namespace render

#endif