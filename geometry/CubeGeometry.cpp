#include "CubeGeometry.h"

#include "../base/Pipeline.h"

#include <random>

namespace gfx1993 {
namespace geometry {

using glm::vec3;
using glm::vec4;
using render::Vertex;

std::vector<Vertex> makeVertices(const vec3 sidelength = vec3(1.f)) {
  std::vector<Vertex> vertices = {
    // Top four vertices.
    Vertex(vec4(-1,  1,  1, 1)),
    Vertex(vec4(-1,  1, -1, 1)),
    Vertex(vec4( 1,  1, -1, 1)),
    Vertex(vec4( 1,  1,  1, 1)),
    // Bottom-four.
    Vertex(vec4(-1, -1,  1, 1)),
    Vertex(vec4(-1, -1, -1, 1)),
    Vertex(vec4( 1, -1, -1, 1)),
    Vertex(vec4( 1, -1,  1, 1))
  };

  vec4 halfSide = vec4(sidelength * 0.5f, 1.0f);
  for (int i = 0; i < 8; ++i) {
    vertices[i].position *= halfSide;
    vertices[i].normal = glm::normalize(vec3(vertices[i].position));

    // TODO: set texture coords.
    vertices[i].color = vec4(1);
  }

  return vertices;
}


Cube Cube::makeSolid(const glm::vec3 &sidelength) {
  Cube cube;
  cube.vertices = makeVertices(sidelength);

  // These are the indices for outside-facing triangle sides.
  // Top +Y
  cube.indices.push_back(0);
  cube.indices.push_back(2);
  cube.indices.push_back(1);
  cube.indices.push_back(0);
  cube.indices.push_back(3);
  cube.indices.push_back(2);
  // -X
  cube.indices.push_back(1);
  cube.indices.push_back(6);
  cube.indices.push_back(5);
  cube.indices.push_back(1);
  cube.indices.push_back(2);
  cube.indices.push_back(6);
  // +Z
  cube.indices.push_back(2);
  cube.indices.push_back(7);
  cube.indices.push_back(6);
  cube.indices.push_back(2);
  cube.indices.push_back(3);
  cube.indices.push_back(7);
  // +X 
  cube.indices.push_back(3);
  cube.indices.push_back(4);
  cube.indices.push_back(7);
  cube.indices.push_back(3);
  cube.indices.push_back(0);
  cube.indices.push_back(4);
  // -Z
  cube.indices.push_back(0);
  cube.indices.push_back(5);
  cube.indices.push_back(4);
  cube.indices.push_back(0);
  cube.indices.push_back(1);
  cube.indices.push_back(5);
  // -Y
  cube.indices.push_back(6);
  cube.indices.push_back(4);
  cube.indices.push_back(5);
  cube.indices.push_back(6);
  cube.indices.push_back(7);
  cube.indices.push_back(4);

  return cube;
}

Cube Cube::makeLines(const vec3& sideLength) {
  Cube cube;
  cube.vertices = makeVertices(sideLength);

  // Line indices for the 8 cube vertices.
  cube.indices = {
    0,1, 1,2, 2,3, 3,0, // top
    4,5, 5,6, 6,7, 7,4, // bottom
    0,4, 1,5, 2,6, 3,7  // connecting sides.
  };

  return cube;
}

}  // namespace geometry
}  // namespace gfx1993
