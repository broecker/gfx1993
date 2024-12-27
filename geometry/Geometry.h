#ifndef GEOMETRY2_INCLUDED
#define GEOMETRY2_INCLUDED

#include "../base/Pipeline.h"

namespace gfx1993 {
namespace geometry {

using render::IndexList;
using render::VertexList;

class Geometry {
public:
  virtual ~Geometry() = default;

  inline const VertexList &getVertices() const { return vertices; }

  VertexList& getMutableVertexList() { return vertices; }

  inline const IndexList &getIndices() const { return indices; }

  // Converts this geometry into flat-shaded by recreating vertices that have
  // per-face normals. This only works on triangle geometries and triples the
  // vertices.
  void makeFlatShaded();

  void makeIndicesForPointCloud();

  void setRandomFaceColors();

  // Access to transform is public -- no reason to write getter+setter
  // for the most-used member.
  glm::mat4 transform = glm::mat4(1.f);

protected:
  // Child classes should write to these two members.
  VertexList vertices;
  IndexList indices;
};

} // namespace geometry
} // namespace gfx1993
#endif
