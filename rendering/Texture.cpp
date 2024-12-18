//
// Created by mbroecker on 4/19/20.
//

#include "Texture.h"

#include <cmath>
#include <iostream>
#include <fstream>
#include <memory>

namespace render {

Texture::Texture(unsigned int width, unsigned int height,
                 const std::string& id) : width(width), height(height), id(id) {
  data = new glm::vec4[width * height];
  for (unsigned int i = 0; i < width * height; ++i) {
    data[i] = glm::vec4(0, 0, 0, 1);
  }
}

Texture::~Texture() { delete[] data; }

const glm::vec4 &Texture::getTexel(const glm::vec2 &texCoords) const {
  // clamp u and v
  float u = glm::clamp(texCoords.x, 0.f, 1.f);
  float v = glm::clamp(texCoords.y, 0.f, 1.f);

  int x = std::floor(u * (width-1));
  int y = std::floor(v * (height-1));

  return getTexel(x, y);
}

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

} // namespace render
