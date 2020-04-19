#ifndef VIEWPORT_INCLUDED
#define VIEWPORT_INCLUDED

#include <glm/glm.hpp>

namespace render
{

    class Viewport
    {
    public:
        Viewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height);


        bool isInside(const glm::ivec2 &p) const;

        glm::vec3 calculateWindowCoordinates(const glm::vec3 &ndc) const;

        glm::ivec2 origin;
        glm::ivec2 size;
    };

}

#endif