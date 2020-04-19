#ifndef FRAMEBUFFER_INCLUDED
#define FRAMEBUFFER_INCLUDED

#include <glm/glm.hpp>
#include <memory>

namespace render
{

    class Framebuffer
    {
    public:
        Framebuffer(unsigned int w, unsigned int h);

        Framebuffer(const Framebuffer &cp);

        virtual ~Framebuffer();

        virtual void clear(const glm::vec4 &c);

        inline void plot(const glm::ivec2 &p, const glm::vec4 &c) { this->plot(p.x, p.y, c); }

        void plot(int x, int y, const glm::vec4 &c);


        inline unsigned int getWidth() const { return width; }

        inline unsigned int getHeight() const { return height; }

        inline const glm::vec4 *getPixels() const { return data; }


    protected:
        unsigned int width, height;
        glm::vec4 *data;

    };

}

#endif