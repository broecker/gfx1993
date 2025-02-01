#include "DemoUtils.h"
#include "Pipeline.h"

#include <glm/gtc/random.hpp>

#include <random>

using glm::vec3;
using glm::vec4;

namespace gfx1993 {

RandomTriangleGeometry::RandomTriangleGeometry(const vec3 &min, const vec3 &max)
    : boundsMin(min), boundsMax(max) {}

void RandomTriangleGeometry::addTriangle() {
  Vertex a = createRandomVertex();
  Vertex b = createRandomVertex();
  Vertex c = createRandomVertex();

  addTriangle(a, b, c);
  addTriangle(a, c, b);
}

void RandomTriangleGeometry::addTriangle(const Vertex &a, const Vertex &b,
                                         const Vertex &c) {
  vertices.push_back(a);
  indices.push_back(vertices.size() - 1);

  vertices.push_back(b);
  indices.push_back(vertices.size() - 1);

  vertices.push_back(c);
  indices.push_back(vertices.size() - 1);

  // Calculate normal for the triangle.
  vec3 u = glm::normalize(b.position - a.position);
  vec3 v = glm::normalize(c.position - a.position);
  vec3 n = glm::normalize(glm::cross(u, v));

  vertices[vertices.size() - 3].normal = n;
  vertices[vertices.size() - 2].normal = n;
  vertices[vertices.size() - 1].normal = n;
}

void RandomTriangleGeometry::clear() {
  indices.clear();
  vertices.clear();
}

Vertex RandomTriangleGeometry::createRandomVertex() const {
  vec3 pos = glm::linearRand(boundsMin, boundsMax);
  vec3 normal = glm::ballRand(1.f);

  glm::vec2 texcoord(0, 0);

  float r = (float)std::rand() / RAND_MAX;
  float g = (float)std::rand() / RAND_MAX;
  float b = 1.f - r - g;

  return Vertex(glm::vec4(pos, 1.f), normal, glm::vec4(r, g, b, 1.f), texcoord);
}

PointField::PointField(std::shared_ptr<HeightMap> heightMap) :
  heightmap(heightMap) {
    for (int x = 0; x < heightMap->getWidth(); x++) {
      for (int z = 0; z < heightMap->getHeight(); z++) {
        Vertex v;
        v.color = glm::vec4(1,0,1,1);
        v.normal = glm::vec3(0);
        v.texcoord = glm::vec2(static_cast<float>(x)/heightMap->getWidth(),
                               static_cast<float>(z)/heightMap->getHeight());

        float y = heightmap->getTexel(glm::ivec2(x, z));
        v.position = glm::vec4(x, y, z, 1);

        vertices.push_back(v);
        indices.push_back(vertices.size()-1);
      }
    }
    assert(indices.size() == vertices.size());
  }

// Assigns each point a color whether it's inside or outside the frustum.
void PointField::updatePoints(const Frustum& f) {
  for (Vertex& v : vertices) {
    vec4 worldSpacePosition = transform * v.position;
    if (f.isInside(glm::vec3(worldSpacePosition))) {
      v.color = glm::vec4(0,1,0,1);
    } else {
      v.color = glm::vec4(1,0,0,1);
    }
  }
}


}  // namespace gfx1993