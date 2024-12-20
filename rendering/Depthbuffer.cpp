#include "Depthbuffer.h"

#include <cassert>
#include <cfloat>

namespace gfx1993 {
namespace render {

Depthbuffer::Depthbuffer(unsigned int w, unsigned int h) : width(w), height(h) {
  data = new float[width * height];
}

Depthbuffer::~Depthbuffer() { delete[] data; }

void Depthbuffer::clear() {
  for (unsigned int i = 0; i < width * height; ++i) {
    data[i] = FLT_MAX;
  }
}

bool Depthbuffer::conditionalPlot(const glm::vec3 &pos) {
  auto x = static_cast<int>(pos.x);
  auto y = static_cast<int>(pos.y);
  float z = pos.z;
  return conditionalPlot(x, y, z);
}

bool Depthbuffer::conditionalPlot(int x, int y, float z) {
  if (x < 0 || y < 0) {
    return false;
  }
  if (static_cast<unsigned int>(x) >= width ||
      static_cast<unsigned int>(y) >= height) {
    return false;
  }

  unsigned int i = x + width * y;
  if (data[i] > z) {
    data[i] = z;
    return true;
  } else
    return false;
}

}  // namespace render
}  // namespace gfx1993