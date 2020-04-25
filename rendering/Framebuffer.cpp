#include "Framebuffer.h"

namespace render
{

    Framebuffer::Framebuffer(unsigned int w, unsigned int h) : width(w), height(h)
    {
        data = new glm::vec4[width * height];
    }

    Framebuffer::~Framebuffer()
    {
        delete[] data;
    }

    void Framebuffer::clear(const glm::vec4 &c)
    {
        for (unsigned int i = 0; i < width * height; ++i) {
            data[i] = c;
        }
    }

    void Framebuffer::plot(int x, int y, const glm::vec4 &c)
    {
        if (x >= 0 && x < width && y >= 0 && y < height) {
            const int index = x + y * width;
            data[index] = c;
        }
    }

}