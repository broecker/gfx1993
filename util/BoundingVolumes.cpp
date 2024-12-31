#include "BoundingVolumes.h"
#include "../base/Pipeline.h"

#include <vector>
#include <iostream>
#include <iomanip>

#include <glm/ext.hpp>

namespace gfx1993 {
namespace util {

using glm::vec3;
using glm::vec4;

BoundingSphere fromGeometry(const geometry::Geometry& geo) {
  // Calculate center.
  vec3 center(0.f);
  for (const render::Vertex& v : geo.getVertices()) {
    center += vec3(v.position);
  }
  center /= static_cast<float>(geo.getVertices().size());

  // Calculate extends.
  float radiusSquared = 0.f;
  for (const render::Vertex& v : geo.getVertices()) {
    vec3 delta = vec3(v.position) - center;
    float dist = dot(delta, delta);
    radiusSquared = glm::max(dist, radiusSquared);    
  }

  return BoundingSphere{center, sqrtf(radiusSquared)};
}

AABB fromVertices(const render::VertexList& vertices) {
  AABB bbox;
  bbox.min = vec3(std::numeric_limits<vec3::value_type>::max());
  bbox.max = vec3(std::numeric_limits<vec3::value_type>::min());

  for (const auto& v : vertices) {
    bbox.min = glm::min(bbox.min, vec3(v.position));
    bbox.max = glm::max(bbox.max, vec3(v.position));
  }

  return bbox;
}

namespace {

constexpr unsigned int maxOctTreeRecursion = 16;

void splitOctTreeRecursively(OctTree* root,
    unsigned int maxVerticesPerNode,
    unsigned int recursionLevel) {

  if (recursionLevel > maxOctTreeRecursion) {
    std::cerr << "[OctTree] Error, max recursion level " << recursionLevel << " reached.\n";
    return;
  }

  if (root->vertices.size() > maxVerticesPerNode) {
    // Create the 8 children, split center. The child indices are:
    // 0 NW top      ^ +x    3 NE top
    // 1 SW top      |       2 SE top
    //               +---> +z
    // 4 NW bottom           7 SE bottom
    // 5 SW bottom           6 SE bottom
    // 
    // top = +y; bottom = -y

    const vec3& max = root->boundingBox.max;
    const vec3& min = root->boundingBox.min;
    const vec3 center = root->boundingBox.getCenter();

    root->children[0] = std::make_unique<OctTree>();
    root->children[0]->boundingBox = { vec3(min.x, center.y, min.z), vec3(max.x, max.y, center.z) };

    root->children[1] = std::make_unique<OctTree>();
    root->children[1]->boundingBox = { vec3(min.x, center.y, center.z), vec3(center.x, max.y, center.z) };

    root->children[2] = std::make_unique<OctTree>();
    root->children[2]->boundingBox = { vec3(min.x, center.y, min.z), vec3(center.x, max.y, max.z) };

    root->children[3] = std::make_unique<OctTree>();
    root->children[3]->boundingBox = {center, root->boundingBox.max};

    root->children[3] = std::make_unique<OctTree>();
    root->children[3]->boundingBox = {center, root->boundingBox.max};

    root->children[4] = std::make_unique<OctTree>();
    root->children[4]->boundingBox = {vec3(center.x, min.y, min.z), vec3(max.x, center.y, center.z) };

    root->children[5] = std::make_unique<OctTree>();
    root->children[5]->boundingBox = {root->boundingBox.min, center};

    root->children[6] = std::make_unique<OctTree>();
    root->children[6]->boundingBox = { vec3(min.x, min.y, center.z), vec3(center.x, center.y, max.z) };

    root->children[7] = std::make_unique<OctTree>();
    root->children[7]->boundingBox = {vec3(center.x, min.y, center.z), vec3(max.x, center.y, max.z)};

    for (const auto& v : root->vertices) {
      for (auto& c : root->children) {
        if (c->boundingBox.isInside(vec3(v.position))) {
          c->vertices.push_back(v);
          //std::cout << "[Oct] Added vertex " << glm::to_string(v.position) << " to bbox " << glm::to_string(c->boundingBox.min) << " - " << glm::to_string(c->boundingBox.max) << std::endl;
        }
      }
    }

    // TODO: We could also sample and store a subset of points here to have some
    // kind of 3d mipmapping/LOD solution.
    root->vertices.clear();

    for (int i = 0; i < 8; ++i) {
      if (root->children[i]->vertices.empty()) {
        // std::cout << "[OctTree] Pruned empty child " << i << std::endl;
        root->children[i] = nullptr;
      } else {
        std::cout << "[Octree] " << std::setw(recursionLevel*2) << recursionLevel << ": " << root->children[i]->vertices.size() << std::endl;

        // TODO: We could also tighten the child bounding boxes here.
        splitOctTreeRecursively(root->children[i].get(), maxVerticesPerNode, recursionLevel+1);
      }
    }
  }
}

}  // namespace

std::unique_ptr<OctTree> fromPointCloud(const render::VertexList& points,
                                        unsigned int maxVerticesPerNode) {
  std::unique_ptr<OctTree> root = std::make_unique<OctTree>();
  root->boundingBox = fromVertices(points);
  root->vertices = points;
  splitOctTreeRecursively(root.get(), maxVerticesPerNode, 0);
  return root;  
}


}  // namespace util
}  // namespace gfx1993
