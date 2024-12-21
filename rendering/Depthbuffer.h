#ifndef DEPTHBUFFER_INCLUDED
#define DEPTHBUFFER_INCLUDED

#include <glm/glm.hpp>

namespace gfx1993 {
namespace render {

class Depthbuffer {
public:
  Depthbuffer(unsigned int w, unsigned int h);

  virtual ~Depthbuffer();

  virtual void clear();

  inline unsigned int getWidth() const { return width; }
  inline unsigned int getHeight() const { return height; }

  inline float getDepth(unsigned int x, unsigned int y) const {
    assert(x <= (width-1));
    assert(y <= (height-1));
    return data[x + width * y];
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
  }

  bool conditionalPlot(const glm::vec3 &pos);

  bool conditionalPlot(int x, int y, float z);

  inline bool isVisible(const glm::ivec2& coord, float depth) {
    return isVisible(coord.x, coord.y, depth);
  }

  inline bool isVisible(int x, int y, float z) const {
    return z < data[x + width * y];
  }

protected:
  unsigned int width, height;
  float *data;
};

} // namespace render
} // namespace gfx1993

#endif