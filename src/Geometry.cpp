#include "Geometry.h"

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/polar_coordinates.hpp>

#include <iostream>
#include <random>

namespace gfx1993 {

using glm::vec2;
using glm::vec3;
using glm::vec4;

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

vec4 getRandomColor() {
  float r = static_cast<float>(std::rand()) / RAND_MAX;
  float g = static_cast<float>(std::rand()) / RAND_MAX;
  
  return vec4(r,g, 1.f-r-g,1.0);
}

void Geometry::setRandomFaceColors() {
  for (size_t i = 0; i < indices.size(); i += 3 ) {
    Vertex& a = vertices[indices[i+0]];
    Vertex& b = vertices[indices[i+1]];
    Vertex& c = vertices[indices[i+2]];

    float r = static_cast<float>(std::rand()) / RAND_MAX;
    float g = static_cast<float>(std::rand()) / RAND_MAX;
   
    vec4 randomColor = getRandomColor();

    a.color = randomColor;
    b.color = randomColor;
    c.color = randomColor;
  }
}

void Geometry::setRandomVertexColors() {
  for (auto& v : vertices) {
    v.color = getRandomColor();
  }
}

void Geometry::makeIndicesForPointCloud() {
  IndexList newIndices;
  for (size_t i = 0; i < vertices.size(); ++i) {
    newIndices.push_back(i);
  }
  indices = newIndices;
}


GridGeometry::GridGeometry() {
  static const int LENGTH = 100;
  static const int STEP = 5;

  static const vec4 MAJOR_COLOR(1.f);
  static const vec4 MINOR_COLOR(0, 0, 0.7f, 1);

  // Create a grid
  for (int x = -LENGTH; x <= LENGTH; x += STEP) {
    Vertex a(vec4(x, 0, -LENGTH, 1));
    Vertex b(vec4(x, 0, LENGTH, 1));

    vec4 color = MINOR_COLOR;
    if (x % (STEP * 10) == 0) {
      color = MAJOR_COLOR;
    }
    a.color = color;
    b.color = color;

    vertices.push_back(a);
    vertices.push_back(b);
    indices.push_back(vertices.size() - 1);
    indices.push_back(vertices.size() - 2);
  }

  for (int z = -LENGTH; z <= LENGTH; z += STEP) {
    Vertex a(vec4(-LENGTH, 0, z, 1));
    Vertex b(Vertex(vec4(LENGTH, 0, z, 1)));

    vec4 color = MINOR_COLOR;
    if (z % (STEP * 10) == 0) {
      color = MAJOR_COLOR;
    }
    a.color = color;
    b.color = color;

    vertices.push_back(a);
    vertices.push_back(b);
    indices.push_back(vertices.size() - 1);
    indices.push_back(vertices.size() - 2);
  }

  for (auto v : vertices) {
    v.normal = vec3(0, 1, 0);
    v.color = vec4(1, 1, 1, 1);
  }
}

Quad::Quad(const vec4 &color) {

  const vec3 normal(0, 0, 1);

  Vertex va = Vertex(vec4(-1, 1, 0, 1), normal, color, vec2(0, 1));
  Vertex vb = Vertex(vec4(-1, -1, 0, 1), normal, color, vec2(0, 0));
  Vertex vc = Vertex(vec4(1, -1, 0, 1), normal, color, vec2(1, 0));
  Vertex vd = Vertex(vec4(1, 1, 0, 1), normal, color, vec2(1, 1));

  vertices =  {va, vb, vc, vd};
  indices = {0,1,2, 2,3,0, 0,2,1, 2,0,3};
}

Quad Quad::makeXZQuad(const glm::vec2& size) {
  Quad q(vec4(1.f));

  const vec2 halfSize = size * 0.5f;
  q.vertices[0].position = vec4(-halfSize.x, 0, -halfSize.y, 1.f);
  q.vertices[1].position = vec4(-halfSize.x, 0,  halfSize.y, 1.f);
  q.vertices[2].position = vec4( halfSize.x, 0,  halfSize.y, 1.f);
  q.vertices[3].position = vec4( halfSize.x, 0, -halfSize.y, 1.f);

  for (int i = 0; i < 4; ++i) {
    q.vertices[i].normal = vec3(0,1,0);
  }

  q.indices = {0,2,1, 2,0,3};

  return q;
}

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

Cube::Cube() {}

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

namespace {
  // How close we want to get to the +/-90.0f poles.
  constexpr float MAX_LATITUDE = 5.f;
  constexpr float LATITUDE_RANGE = 180.f - 2*MAX_LATITUDE;
}

static Vertex makeSphereVertex(float phi, float theta, float radius) {
  vec2 sphericalCoords = vec2((float)theta, phi);

  glm::vec3 pos =
      glm::euclidean(glm::radians(sphericalCoords)) * radius;
  glm::vec3 normal = glm::normalize(pos);
  glm::vec2 texcoord = glm::vec2(phi, theta) / glm::vec2(90, 360);
  glm::vec4 color = glm::vec4(1);

  return Vertex(glm::vec4(pos, 1), normal, color, texcoord);
}

Sphere::Sphere(float radius, unsigned int latitudes, unsigned int longitudes) {
  assert(radius > 0.f);
  assert(latitudes > 2);
  assert(longitudes > 2);

  for (unsigned int lat = 0; lat < latitudes; ++lat) {
    float theta = (static_cast<float>(lat) / (latitudes-1) * LATITUDE_RANGE - 90) + MAX_LATITUDE;
    // A full circle on the horizontal plane.
    for (unsigned int lon = 0; lon < longitudes; ++lon) {
      float phi = static_cast<float>(lon) / longitudes * 360.f;
      vertices.push_back(makeSphereVertex(phi, theta, radius));
    }
  }

  // At this point we have a stack of Latitude circles, each with Longitude
  // points. We can use 2D -> 1D index lookups, similar to what we do with
  // images.
  for (unsigned int lat = 0; lat < latitudes-1; ++lat) {
    for (unsigned int lon = 0; lon < longitudes; ++lon) {
      size_t a = lon + lat*longitudes;
      size_t b = lon + (lat+1)*longitudes;
      size_t c = lon+1 + (lat+1)*longitudes;
      size_t d = lon+1 + lat*longitudes;

      if (lon == longitudes-1) {
        c = 0 + (lat+1)*longitudes;
        d = 0 + lat*longitudes;
      }

      indices.push_back(a);
      indices.push_back(c);
      indices.push_back(b);

      indices.push_back(a);
      indices.push_back(d);
      indices.push_back(c);
    }
  }

  // Add south and north pole.
  vertices.push_back(makeSphereVertex(0, -90, radius));
  size_t southPoleIdx = vertices.size()-1;
  for (unsigned int lon = 0; lon < longitudes; ++lon) {
    if (lon == longitudes-1) {
      indices.push_back(lon);
      indices.push_back(southPoleIdx);
      indices.push_back(0);
    } else {
      indices.push_back(lon);
      indices.push_back(southPoleIdx);
      indices.push_back(lon+1);
    }
  }
  
  vertices.push_back(makeSphereVertex(0, 90, radius));
  size_t northPoleIdx = vertices.size()-1;
  size_t idxOffset = (latitudes-1)*longitudes;
  for (unsigned int lon = 0; lon < longitudes; ++lon) {
    if (lon == longitudes-1) {
      indices.push_back(idxOffset + lon);
      indices.push_back(idxOffset + 0);
      indices.push_back(northPoleIdx);
    } else {
      indices.push_back(idxOffset + lon);      
      indices.push_back(idxOffset + lon+1);
      indices.push_back(northPoleIdx);
    }
  }
}


}  // namespace gfx1993