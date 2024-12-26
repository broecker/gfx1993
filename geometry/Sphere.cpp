#include "Sphere.h"

#include <glm/ext.hpp>
#include <glm/gtx/polar_coordinates.hpp>
#include <glm/gtc/random.hpp>

#include <cassert>
#include <iostream>
#include <random>

namespace gfx1993 {
namespace geometry {

using glm::vec2;
using glm::vec3;
using glm::vec4;
using render::Vertex;

// How close we want to get to the +/-90.0f poles.
constexpr float MAX_LATITUDE = 5.f;
constexpr float LATITUDE_RANGE = 180.f - 2*MAX_LATITUDE;

static Vertex makeSphereVertex(float phi, float theta, float radius) {
  vec2 sphericalCoords = vec2((float)theta, phi);

  glm::vec3 pos =
      glm::euclidean(glm::radians(sphericalCoords)) * radius;
  glm::vec3 normal = glm::normalize(pos);
  glm::vec2 texcoord = glm::vec2(phi, theta) / glm::vec2(90, 360);
  glm::vec4 color = glm::vec4(1);

  return render::Vertex(glm::vec4(pos, 1), normal, color, texcoord);
}

Sphere::Sphere(float radius, unsigned int latitudes, unsigned int longitudes) {
  assert(radius > 0.f);
  assert(latitudes > 2);
  assert(longitudes > 2);

  for (unsigned int lat = 0; lat < latitudes; ++lat) {

    float theta = (static_cast<float>(lat) / (latitudes-1) * LATITUDE_RANGE - 90) + MAX_LATITUDE;
    std::cout << "lat: " << lat << " t: " << theta << std::endl;

    float theta1 = (static_cast<float>(lat+1) / (latitudes-1) * LATITUDE_RANGE - 90) + MAX_LATITUDE;
    std::cout << "lat: " << lat << " t1: " << theta1 << std::endl;

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

} // namespace geometry
} // namespace gfx1993