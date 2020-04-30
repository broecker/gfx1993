#ifndef RASTERISER_INCLUDED
#define RASTERISER_INCLUDED

#include <memory>

#include "Clipper.h"
#include "Pipeline.h"
#include "RenderConfig.h"
#include "RenderDebugInfo.h"

namespace render {
// Main class that does the heavy lifting in putting fragments into the
// framebuffer.
class Rasterizer {
public:
  virtual ~Rasterizer() = default;

  // Draws the vertices as points. Only the indexed vertices are drawn.
  void drawPoints(const RenderConfig &renderConfig,
                          const VertexList &vertices,
                          const IndexList &indices) const;

  // Draws the vertices as lines. Every two indices define the endpoints of a
  // line.
  void drawLines(const RenderConfig &renderConfig,
                         const VertexList &vertices,
                         const IndexList &indices) const;

  // Draws a continuous line strip; each index is considered to be the either
  // the midpoint between two lines, or the endpoint in case of the first and
  // last index. To draw a closed loop, make sure that the first and last index
  // are the same; i.e. [0, 1, 2, 3, 0]
  void drawLineStrip(const RenderConfig &renderConfig,
                             const VertexList &vertices,
                             const IndexList &indices) const;

  // Draws the vertices as lines. Every three indices are treated as the three
  // corners of a triangle. Triangles are defined counter-clockwise.
  void drawTriangles(const RenderConfig &renderConfig,
                             const VertexList &vertices,
                             const IndexList &indices) const;

  inline void resetDebugInfo() { debugInfo.reset(); }

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

  // Rasterizes a single fragment to the buffer after performing depth test and
  // alpha blending. This is called from both the drawTriangle and drawLine
  // methods.
  void drawFragment(const RenderConfig &renderConfig,
                    const ShadingGeometry &geometry) const;

  Clipper             clipper;
  mutable DebugInfo   debugInfo;
};

} // namespace render

#endif