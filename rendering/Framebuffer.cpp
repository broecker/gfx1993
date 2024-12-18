#include "Framebuffer.h"

namespace render {

Framebuffer::Framebuffer(unsigned int w, unsigned int h) : width(w), height(h), data(w*h) {}

void Framebuffer::clear(const glm::vec4 &c) {
  for (unsigned int i = 0; i < width * height; ++i) {
    data[i] = c;
  }
}

void Framebuffer::plot(unsigned int x, unsigned int y, const glm::vec4 &c) {
  if (x >= 0 && x < width && y >= 0 && y < height) {
    const int index = x + y * width;
    data[index] = c;
  }
}

const glm::vec4& Framebuffer::getPixel(const glm::vec2& p) const {
  // clamp u and v
  float u = glm::clamp(p.x, 0.f, 1.f);
  float v = glm::clamp(p.y, 0.f, 1.f);

  int x = std::floor(u * (width-1));
  int y = std::floor(v * (height-1));

  return getPixel(x, y);
}

void Framebuffer::fillUint8RgbaBuffer(uint8_t* buffer) const {
  size_t idx = 0;
  for (unsigned int i = 0; i < width*height; ++i) {
    const auto& c = data[i];
    buffer[idx+0] = static_cast<uint8_t>(c.r * 255);
    buffer[idx+1] = static_cast<uint8_t>(c.g * 255);
    buffer[idx+2] = static_cast<uint8_t>(c.b * 255);
    buffer[idx+3] = static_cast<uint8_t>(c.a * 255);
    idx += 4;
  }
}

std::vector<uint8_t> Framebuffer::getUint8RgbaBuffer() const {
  std::vector<uint8_t> result(width*height*4);
  fillUint8RgbaBuffer(&result[0]);
  return result;
}

}