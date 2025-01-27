#include "Teapot.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <fstream>
#include <iostream>

namespace gfx1993 {

constexpr char teapotFile[] = "../models/teapot_5144.tris";

Teapot::Teapot() {

  std::ifstream file(teapotFile);
  if (!file.is_open()) {
    std::cerr << "[Teapot] Unable to open file \"" << teapotFile << "\"\n";
    return;
  } 
      
  std::clog << "[Teapot] Loading file " << teapotFile << std::endl;

  std::string buffer;

  // The first line contains the number of triangles;
  std::getline(file, buffer);
  unsigned int triangles = atoi(buffer.c_str());

  std::clog << "[Teapot] Reading " << triangles << " triangles.\n";

  vertices.reserve(triangles*3);
  indices.reserve(triangles*3);

  //0: vertex 0; 1: normal 0; 2: vertex 1; 3: normal 1; 4: vertex 2; 5: normal 2
  int counter = 0;
  glm::vec3 vecBuffer[6];

  while (!file.eof()) {
    std::getline(file, buffer);

    if (buffer.empty())
      continue;

    glm::vec3 v;
    assert(sscanf(buffer.c_str(), "%f %f %f", &v.x, &v.y, &v.z) == 3);

    vecBuffer[counter++] = v;
    if (counter == 6) {
      counter = 0;
      
      Vertex v0;
      v0.position = glm::vec4(vecBuffer[0], 1.f);
      v0.normal = glm::normalize(vecBuffer[1]);
      vertices.push_back(v0);
      indices.push_back(vertices.size()-1);

      Vertex v1;
      v1.position = glm::vec4(vecBuffer[2], 1.f);
      v1.normal = glm::normalize(vecBuffer[3]);
      vertices.push_back(v1);
      indices.push_back(vertices.size()-1);

      Vertex v2;
      v2.position = glm::vec4(vecBuffer[4], 1.f);
      v2.normal = glm::normalize(vecBuffer[5]);
      vertices.push_back(v2);
      indices.push_back(vertices.size()-1);
    }
  }
}

}  // namespace gfx1993