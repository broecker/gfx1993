#ifndef GFX1993_DEMOUTILS_INCLUDED
#define GFX1993_DEMOUTILS_INCLUDED

#include "Frustum.h"
#include "Geometry.h"
#include "Texture.h"

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

// Creates a simple pointfield based on a height map.
class PointField : public Geometry {
public:
  PointField(unsigned int width, unsigned int height);
  PointField(std::shared_ptr<HeightMap> heightMap);

  // Assigns each point a color whether it's inside or outside the frustum.
  void updatePoints(const Frustum& f);
private:
  std::shared_ptr<HeightMap> heightmap;
};

// The Utah Teapot.
// See https://users.cs.utah.edu/~dejohnso/models/teapot.html
class Teapot : public Geometry {
public:
  Teapot();
};

}  // namespace gfx1993

#endif GFX1993_DEMOUTILS_INCLUDED
