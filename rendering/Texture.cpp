//
// Created by mbroecker on 4/19/20.
//

#include "Texture.h"

#include <cmath>
#include <fstream>

namespace render{

    Texture::Texture(int width, int height, const glm::vec4& fillColor) : width(width), height(height)
    {
        data = new glm::vec4[width*height];
        for (int i = 0; i < width*height; ++i) {
            data[i] = fillColor;
        }
    }

    Texture::~Texture() {
        delete[] data;
    }

    const glm::vec4& Texture::getTexel(const glm::vec2& texCoords) const {
        // clamp u and v
        float u = glm::clamp(texCoords.x, 0.f, 1.f);
        float v = glm::clamp(texCoords.y, 0.f, 1.f);

        int x = std::ceil(u * width);
        int y = std::ceil(v * height);

        return getTexel(x, y);
    }

    void Texture::makeCheckerboard(const glm::vec4& a, const glm::vec4& b, int checkerSize)
    {
        int color = 0;
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {

                // Switch colors
                if (x % checkerSize == 0) {
                    color = 1 - color;
                }

                if (color == 1) {
                    setTexel(x, y, a);
                } else {
                    setTexel(x, y, b);
                }
            }

            if (y % checkerSize == 0) {
                color = 1 - color;
            }
        }
    }

    bool Texture::loadPPM(const std::string &filename) {
        std::ifstream file(filename);

        if (!file.is_open()) {
            return false;
        }

        std::string header;
        file >> header;

        if (header != "P3") {
            return false;
        }

        int w, h;
        file >> w >> h;
        if (w != width || h != height) {
            width = w;
            height = h;
            delete[] data;
            data = new glm::vec4[width*height];
        }

        for (int i = 0; i < width*height; ++i)
        {
            int r, g, b;
            file >> r >> g >> b;
            data[i] = glm::vec4(glm::vec3(r, g, b) / 255.f, 1);
        }

        return true;
    }

}
