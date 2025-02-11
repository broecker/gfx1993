#ifndef GEOMETRY2_INCLUDED
#define GEOMETRY2_INCLUDED

#include "Pipeline.h"

namespace gfx1993 {

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

  void setRandomVertexColors();

  // Access to transform is public -- no reason to write getter+setter
  // for the most-used member.
  glm::mat4 transform = glm::mat4(1.f);

protected:
  // Child classes should write to these two members.
  VertexList vertices;
  IndexList indices;
};

// A 2D grid on the XZ plane.
class GridGeometry : public Geometry {
public:
  GridGeometry();
};

// A three-dimensional cube with solid faces.
class Cube : public Geometry {
public:
  static Cube makeSolid(const glm::vec3& sideLength = glm::vec3(1.f));
  static Cube makeLines(const glm::vec3& sideLength = glm::vec3(1.f));

private:
  Cube();
};

class Quad : public Geometry {
public:
  // Constructs a quad from 4 vertices. Assumes the following geometry:
  // a +---+ d
  //   | \ |
  // b +---+ c
  // The size will be [-1..1] along the XY axis and 0 on the z axis. It can be
  // used for screen-space rendering.
  // The quad is double-sided.
  explicit Quad(const glm::vec4 &color);

  // Single-size quad with the given size. The center point will be at (0,0,0).
  static Quad makeXZQuad(const glm::vec2& size);
};

class Sphere : public Geometry {
public:
  Sphere(float radius, unsigned int latitudes, unsigned int longitudes);
};


} // namespace gfx1993

#endif  // GEOMETRY2_INCLUDED