#include "Geometry.h"

#include <glm/glm.hpp>

#include <iostream>
#include <random>

namespace gfx1993 {
namespace geometry {

using glm::vec3;
using glm::vec4;
using render::Vertex;

void Geometry::makeFlatShaded() {
  if (indices.size() % 3 != 0) {
    std::cerr << "[Geometry] Invalid geometry, expected triangles.\n";
    return;
  }

  IndexList newIndices;
  VertexList newVertices;

  for (size_t i = 0; i < indices.size(); i += 3 ) {
    Vertex a = vertices[indices[i+0]];
    Vertex b = vertices[indices[i+1]];
    Vertex c = vertices[indices[i+2]];

    vec3 normal = glm::normalize(
                    glm::cross(vec3(b.position - a.position), 
                                 vec3(c.position - a.position)));

    a.normal = b.normal = c.normal = normal;

    newVertices.push_back(a);
    newIndices.push_back(newVertices.size()-1);
    newVertices.push_back(b);
    newIndices.push_back(newVertices.size()-1);
    newVertices.push_back(c);
    newIndices.push_back(newVertices.size()-1);
  }

  vertices = newVertices;
  indices = newIndices;

  std::cout << "[Geometry] New geometry: " << vertices.size() << " vertices, " << indices.size() << " indices.\n";
}

void Geometry::setRandomFaceColors() {
  for (size_t i = 0; i < indices.size(); i += 3 ) {
    Vertex& a = vertices[indices[i+0]];
    Vertex& b = vertices[indices[i+1]];
    Vertex& c = vertices[indices[i+2]];

    float r = static_cast<float>(std::rand()) / RAND_MAX;
    float g = static_cast<float>(std::rand()) / RAND_MAX;
   
    vec4 randomColor(r,g, 1.f-r-g,1.0);

    a.color = randomColor;
    b.color = randomColor;
    c.color = randomColor;
  }
}

void Geometry::makeIndicesForPointCloud() {
  IndexList newIndices;
  for (size_t i = 0; i < vertices.size(); ++i) {
    newIndices.push_back(i);
  }
  indices = newIndices;
}




}  // namespace geometry
}  // namespace gfx1993