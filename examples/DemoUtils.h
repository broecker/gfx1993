#ifndef GFX1993_DEMOUTILS_INCLUDED
#define GFX1993_DEMOUTILS_INCLUDED

#include "Geometry.h"

#include <glm/glm.hpp>

namespace gfx1993 {

class RandomTriangleGeometry : public Geometry {
public:
  RandomTriangleGeometry(const glm::vec3 &boundsMin,
                         const glm::vec3 &boundsMax);

  // Clears all contained triangles.
  void clear();

  // Adds a double sided triangle.
  void addTriangle();

private:
  glm::vec3 boundsMin, boundsMax;

  Vertex createRandomVertex() const;

  void addTriangle(const Vertex &a, const Vertex &b, const Vertex &c);
};

}  // namespace gfx1993

#endif GFX1993_DEMOUTILS_INCLUDED
