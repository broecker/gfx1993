#ifndef RASTERISER_INCLUDED
#define RASTERISER_INCLUDED

#include <memory>

#include "Clipper.h"
#include "../base/Pipeline.h"
#include "RenderConfig.h"
#include "../util/RenderDebugInfo.h"
#include "../util/RenderProfileInfo.h"

namespace gfx1993 {
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

  // Fills the screen in coordinates [0..1] with the given fragment shader.
  // Because no actual vertices are drawn, no vertex shader is used.
  void drawScreenFillingQuad(const RenderConfig &renderConfig);

  inline void resetDebugInfo() { debugInfo.reset(); profileInfo.reset(); }

  inline util::RenderProfile& getProfile() { return profileInfo; }
  inline util::DebugInfo& getDebugInfo() { return debugInfo; }

  const Clipper& getClipper() const { return clipper; }

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
  // methods. Returns whether the pixel was actually drawn or not.
  bool drawFragment(const RenderConfig &renderConfig,
                    const ShadingGeometry &geometry) const;

  // Rasterizes a single fragment /only/ to the depth buffer.
  bool drawDepthFragment(const RenderConfig &renderConfig,
                         const ShadingGeometry &geometry) const;

  Clipper               clipper;
  mutable util::DebugInfo     debugInfo;
  mutable util::RenderProfile profileInfo;
};

} // namespace render
} // namespace gfx1993

#endif