//
// Created by mbroecker on 4/19/20.
//
#ifndef GFX1993_TEXTURE_H
#define GFX1993_TEXTURE_H

#include <memory>

#include <glm/glm.hpp>

namespace render
{

    class Texture
    {
    public:
        virtual ~Texture();

        const glm::vec4 &getTexel(const glm::vec2 &texCoords) const;

        static std::unique_ptr<Texture> makeFlat(int width, int height, const glm::vec4 &fillColor);

        static std::unique_ptr<Texture>
        makeCheckerboard(int width, int height, int checkerSize, const glm::vec4 &a, const glm::vec4 &b);

        static std::unique_ptr<Texture> loadPPM(const std::string &filename);

    private:
        int width, height;
        glm::vec4 *data;

        explicit Texture(int width, int height);

        inline const glm::vec4 &getTexel(int x, int y) const
        {
            return data[x + y * width];
        }

        inline void setTexel(int x, int y, const glm::vec4 &c)
        {
            data[x + y * width] = c;
        }
    };

}

#endif //GFX1993_TEXTURE_H
