#include "Texture.h"

#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <memory>

#include <glm/ext.hpp>
#include <glm/gtc/noise.hpp>

namespace gfx1993 {
namespace render {

std::unique_ptr<Texture> Texture::makeFlat(unsigned int width, unsigned int height,
                                           const glm::vec4 &fillColor) {
  // Constructor is private so we cannot call make_unique...
  std::unique_ptr<Texture> texture(new Texture(width, height, "Flat"));

  for (unsigned int i = 0; i < width * height; ++i) {
    texture->data[i] = fillColor;
  }

  return texture;
}

std::unique_ptr<Texture> Texture::makeCheckerboard(unsigned int width, 
                                                   unsigned int height,
                                                   unsigned int checkerSize,
                                                   const glm::vec4 &a,
                                                   const glm::vec4 &b) {
  // Constructor is private so we cannot call make_unique...
  std::unique_ptr<Texture> texture(new Texture(width, height, "Checkerboard"));

  int color = 0;
  for (unsigned int y = 0; y < height; ++y) {
    for (unsigned int x = 0; x < width; ++x) {

      // Switch colors
      if (x % checkerSize == 0) {
        color = 1 - color;
      }

      if (color == 1) {
        texture->setTexel(x, y, a);
      } else {
        texture->setTexel(x, y, b);
      }
    }

    if (y % checkerSize == 0) {
      color = 1 - color;
    }
  }

  return texture;
}

std::unique_ptr<Texture> Texture::loadPPM(const std::string &filename) {
  std::cout << "[Texture] Loading file " << filename << " ... \n";

  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "[Texture] Unable to open file.\n";
    return nullptr;
  }

  std::string header;
  file >> header;

  if (header != "P3") {
    std::cerr << "[Texture] Invalid PPM header.\n";
    return nullptr;
  }

  int width, height, maxVal;
  file >> width >> height >> maxVal;

  // Constructor is private so we cannot call make_unique...
  std::unique_ptr<Texture> texture(new Texture(width, height, filename));

  std::cout << "[Texture] " << width << "x" << height << "; setting data.\n";
  for (int i = 0; i < width * height; ++i) {
    int r, g, b;
    file >> r >> g >> b;
    texture->data[i] = glm::vec4(glm::vec3(r, g, b) / (float)maxVal, 1);
  }

  std::cout << "[Texture] Loaded successfully.\n";
  return texture;
}

std::unique_ptr<Texture> Texture::fromVec4s(unsigned int width, unsigned int height, const std::vector<glm::vec4>& data) {
  if (width*height != data.size()) {
    std::cerr << "[Texture] data size (" << data.size() << ") does not match texture dimensions " << width << "x" << height << "!\n";
    return nullptr;
  }

  std::unique_ptr<Texture> texture(new Texture(width, height, "Raw"));
  memcpy(reinterpret_cast<void*>(&texture->data[0]), &data[0], sizeof(glm::vec4)*width*height);

  return texture;
}

std::unique_ptr<Texture> Texture::perlinNoise(unsigned int width, unsigned int height, const glm::vec2& scale) {
  std::unique_ptr<Texture> texture(new Texture(width, height, "Noise"));

  for (unsigned int w = 0; w < width; ++w) {
    for (unsigned int h = 0; h < height; ++h) {
      double x = static_cast<double>(w) / width;
      double y = static_cast<double>(h) / height;

      texture->setTexel(w, h, glm::vec4(glm::perlin(glm::vec3(x * scale.x, y * scale.y, 0.f))));
    }
  }
  return texture;
}

std::unique_ptr<HeightMap> HeightMap::makeFlat(unsigned int x, unsigned int z) {
  std::unique_ptr<HeightMap> heightmap(new HeightMap(x, z, "Flat"));
  float fill = 0.f;
  heightmap->data.resize(x*z, fill);
  heightmap->maxHeight = heightmap->minHeight = fill;
  return heightmap;
}

std::unique_ptr<HeightMap> HeightMap::perlinNoise(unsigned int width, unsigned int height, const glm::vec3& scale, const glm::vec3& offset) {
  std::unique_ptr<HeightMap> heightmap(new HeightMap(width, height, "Noise"));
  for (unsigned int w = 0; w < width; ++w) {
    for (unsigned int h = 0; h < height; ++h) {
      double x = static_cast<double>(w) / width;
      double z = static_cast<double>(h) / height;

      glm::vec3 pos = glm::vec3(x, 0.f, z) + offset;
      float y = glm::perlin(pos * scale) * scale.y;

      heightmap->setHeight(w, h, y);
    }
  }

  std::cout << "[Heightmap] Created " << width << "x" << height << " perlin noise heightmap; " << heightmap->minHeight << "-" << heightmap->maxHeight << std::endl;

  return heightmap;
}



} // namespace render
} // namespace gfx1993