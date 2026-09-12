#include <GUnit.h>

#include <cstdio>
#include <fstream>
#include <limits>
#include <string>

#include <unistd.h>

#include <glm/ext.hpp>
#include <glm/gtc/epsilon.hpp>

#include "PlyGeometry.h"

namespace gfx1993 {
namespace {

using glm::vec3;

// Deliberately not named `equal` -- unqualified `equal(vec, vec)` is
// ambiguous with (and, per partial ordering, loses to) glm::equal(vec, vec),
// which returns a component-wise vec<L,bool> instead of a bool. That
// silently turns EXPECT(equal(a, b)) into a no-op assertion that always
// reports success, since GUnit's EXPECT never gets a genuine bool.
template <class glm_type>
inline bool approxEqual(const glm_type &a, const glm_type &b) {
  return glm::all(glm::epsilonEqual(a, b, 1e-4f));
}

// A flat two-triangle quad in the XY plane (z=0), spanning (0,0,0) to
// (2,2,0). Both triangles wind the same way, so every vertex normal should
// come out as (0,0,1) once renormalized -- and after center(), the quad
// should be symmetric around the origin.
const char *kQuadPly = R"(ply
format ascii 1.0
element vertex 4
property float x
property float y
property float z
property float confidence
property float intensity
element face 2
property list uchar int vertex_indices
end_header
0 0 0 1 1
2 0 0 1 1
2 2 0 1 1
0 2 0 1 1
3 0 1 2
3 0 2 3
)";

std::string writeFixture(const char *contents) {
  std::string path =
      "/tmp/gfx1993_plygeometry_test_" + std::to_string(getpid()) + ".ply";
  std::ofstream out(path);
  out << contents;
  return path;
}

}  // namespace

GTEST("PlyGeometry Test") {
  SHOULD("Load vertex and index counts from the header") {
    std::string path = writeFixture(kQuadPly);

    PlyGeometry geo;
    ASSERT(geo.loadPly(path));

    EXPECT(geo.getVertices().size() == 4);
    EXPECT(geo.getIndices().size() == 6);

    std::remove(path.c_str());
  }

  SHOULD("Renormalize vertex normals after loading") {
    // Regression test: loadPly()'s normal-renormalization loop used to
    // iterate `for (auto v : vertices)` (by value), so the normalize() never
    // wrote back -- normals stayed as the raw accumulated (non-unit-length)
    // sums from the per-face pass instead of being unit vectors.
    std::string path = writeFixture(kQuadPly);

    PlyGeometry geo;
    ASSERT(geo.loadPly(path));

    for (const auto &v : geo.getVertices()) {
      EXPECT(approxEqual(v.normal, vec3(0, 0, 1)));
    }

    std::remove(path.c_str());
  }

  SHOULD("Actually move vertices when centering") {
    // Regression test: center()'s recentering loop had the same by-value
    // iteration bug -- `for (auto v : vertices)` never wrote the recomputed
    // position back, so center() was a complete no-op.
    std::string path = writeFixture(kQuadPly);

    PlyGeometry geo;
    ASSERT(geo.loadPly(path));
    geo.center();

    // Original bounding box was [0,0,0] .. [2,2,0]; centering should move it
    // to be symmetric around the origin, i.e. [-1,-1,0] .. [1,1,0].
    const VertexList &vertices = geo.getVertices();
    ASSERT(vertices.size() == 4);
    EXPECT(approxEqual(vec3(vertices[0].position), vec3(-1, -1, 0)));
    EXPECT(approxEqual(vec3(vertices[1].position), vec3(1, -1, 0)));
    EXPECT(approxEqual(vec3(vertices[2].position), vec3(1, 1, 0)));
    EXPECT(approxEqual(vec3(vertices[3].position), vec3(-1, 1, 0)));

    std::remove(path.c_str());
  }

  SHOULD("Fail gracefully on a missing file") {
    PlyGeometry geo;
    EXPECT(!geo.loadPly("/nonexistent/path/that/should/not/exist.ply"));
  }
}

}  // namespace gfx1993
