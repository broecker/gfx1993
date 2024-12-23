//
// Created by mbroecker on 4/19/20.
//

#include "Texture.h"

#include <cmath>
#include <cstring>
#include <iostream>
#include <fstream>
#include <memory>

namespace gfx1993 {
namespace render {

Texture::Texture(unsigned int width, unsigned int height,
                 const std::string& id) : width(width), height(height), id(id) {
  data = new glm::vec4[width * height];
  for (unsigned int i = 0; i < width * height; ++i) {
    data[i] = glm::vec4(0, 0, 0, 1);
  }
}

Texture::~Texture() { delete[] data; }

const glm::vec4 &Texture::getTexel(const glm::vec2 &texCoords, LookupMode mode) const {
  float u,v;
  if (mode == CLAMP) { 
    // clamp u and v
    u = glm::clamp(texCoords.x, 0.f, 1.f);
    v = glm::clamp(texCoords.y, 0.f, 1.f);
  }

  if (mode == REPEAT) {
    u = glm::fract(texCoords.x);
    v = glm::fract(texCoords.y);
  }

  unsigned int x = std::floor(u * (width-1));
  unsigned int y = std::floor(v * (height-1));

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

std::unique_ptr<Texture> Texture::fromVec4s(unsigned int width, unsigned int height, const std::vector<glm::vec4>& data) {
  if (width*height != data.size()) {
    std::cerr << "[Texture] data size (" << data.size() << ") does not match texture dimensions " << width << "x" << height << "!\n";
    return nullptr;
  }

  std::unique_ptr<Texture> texture(new Texture(width, height, "Raw"));
  memcpy(texture->data, &data[0], sizeof(glm::vec4)*width*height);

  return texture;
}

// Adapted from https://cs.nyu.edu/~perlin/noise/
namespace noise {

constexpr int permutation[] = { 151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
  };

static int p[512];
static bool p_initialized = false; 

static double fade(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }
static double lerp(double t, double a, double b) { return a + t * (b - a); }
static double grad(int hash, double x, double y, double z) {
  int h = hash & 15;                      // CONVERT LO 4 BITS OF HASH CODE
  double u = h<8 ? x : y,                 // INTO 12 GRADIENT DIRECTIONS.
          v = h<4 ? y : h==12||h==14 ? x : z;
  return ((h&1) == 0 ? u : -u) + ((h&2) == 0 ? v : -v);
}

static double perlin(double x, double y, double z) {
  if (!p_initialized) {
    for (int i=0; i < 256 ; i++) {
      p[256+i] = p[i] = permutation[i]; 
    }
    p_initialized = true;
  }

  int X = (int)floor(x) & 255,                  // FIND UNIT CUBE THAT
      Y = (int)floor(y) & 255,                  // CONTAINS POINT.
      Z = (int)floor(z) & 255;
      x -= floor(x);                            // FIND RELATIVE X,Y,Z
      y -= floor(y);                            // OF POINT IN CUBE.
      z -= floor(z);
  double u = fade(x),                           // COMPUTE FADE CURVES
         v = fade(y),                           // FOR EACH OF X,Y,Z.
         w = fade(z);
  int A = p[X  ]+Y, AA = p[A]+Z, AB = p[A+1]+Z,      // HASH COORDINATES OF
      B = p[X+1]+Y, BA = p[B]+Z, BB = p[B+1]+Z;      // THE 8 CUBE CORNERS,

  return lerp(w, lerp(v, lerp(u, grad(p[AA  ], x, y, z), // AND ADD
                 grad(p[BA  ], x-1, y  , z   )), // BLENDED
                 lerp(u, grad(p[AB  ], x  , y-1, z   ),  // RESULTS
                 grad(p[BB  ], x-1, y-1, z   ))),// FROM  8
                 lerp(v, lerp(u, grad(p[AA+1], x  , y  , z-1 ),  // CORNERS
                 grad(p[BA+1], x-1, y  , z-1 )), // OF CUBE
                 lerp(u, grad(p[AB+1], x  , y-1, z-1 ),
                 grad(p[BB+1], x-1, y-1, z-1 ))));
}

}  // namespace noise

std::unique_ptr<Texture> Texture::perlinNoise(unsigned int width, unsigned int height, const glm::vec2& scale) {
  std::unique_ptr<Texture> texture(new Texture(width, height, "Noise"));

  for (unsigned int w = 0; w < width; ++w) {
    for (unsigned int h = 0; h < height; ++h) {
      double x = static_cast<double>(w) / width;
      double y = static_cast<double>(h) / height;

      texture->setTexel(w, h, glm::vec4(noise::perlin(x * scale.x, y * scale.y, 0)));
    }
  }

  return texture;
}

} // namespace render
} // namespace gfx1993