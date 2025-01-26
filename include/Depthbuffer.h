#ifndef DEPTHBUFFER_INCLUDED
#define DEPTHBUFFER_INCLUDED

#include "config.h"

#include <glm/glm.hpp>
#include <limits>
#include <vector>

namespace gfx1993 {
namespace render {

class Depthbuffer {
public:
  Depthbuffer(unsigned int w, unsigned int h);

  virtual ~Depthbuffer() = default;

  virtual void clear(float depth);
  inline void clear() {
    clear(std::numeric_limits<float>::max());
  }

  inline unsigned int getWidth() const { return width; }
  inline unsigned int getHeight() const { return height; }

  inline float getDepth(unsigned int x, unsigned int y) const {
    assert(x <= (width-1));
    assert(y <= (height-1));
    return data[x + width * y];
  }

  inline float getDepth(const glm::ivec2& t) const {
    return getDepth(t.x, t.y);
  }

  inline float getDepth(const glm::vec2& t) const {
    assert(t.x >= 0 && t.x < 1.0);
    assert(t.y >= 0 && t.y < 1.0);
    return getDepth(t.x*width, t.y*width);
  }

  inline void plot(const glm::ivec2& coords, float depth) {
    plot(coords.x, coords.y, depth);
  }

  inline void plot(unsigned int x, unsigned int y, float z) {
    data[x + width * y] = z;
#if GFX1993_DEPTHBUFFER_LOG_WRITES
    depthWrites[x + width * y]++;
#endif
  }

  bool conditionalPlot(const glm::vec3 &pos);

  bool conditionalPlot(int x, int y, float z);

  inline bool isVisible(const glm::ivec2& coord, float depth) {
    return isVisible(coord.x, coord.y, depth);
  }

  inline bool isVisible(int x, int y, float z) const {
    return z < data[x + width * y];
  }

  inline unsigned int getDepthWrites(unsigned int x, unsigned int y) const {
    assert(x <= (width-1));
    assert(y <= (height-1));
    return depthWrites[x + width * y];
  }

  inline unsigned short getMaxDepthWrites() const {
    unsigned short max = 0;
    for (size_t i = 0; i < depthWrites.size(); ++i) {
      max = glm::max(max, depthWrites[i]);
    }
    return max;
  }

  inline float getMaxDepth() const {
    float max = std::numeric_limits<float>::max();
    for (size_t i = 0; i < data.size(); ++i) {
      max = glm::min(max, data[i]);
    }
    return max;
  }

protected:
  unsigned int width, height;
  std::vector<float> data;

  // This is only filled when GFX1993_DEPTHBUFFER_LOG_WRITES is set.
  mutable std::vector<unsigned short> depthWrites;
};

} // namespace render
} // namespace gfx1993

#endif