#ifndef DEPTHBUFFER_INCLUDED
#define DEPTHBUFFER_INCLUDED

#include <glm/glm.hpp>

namespace render
{

    class Depthbuffer
    {
    public:
        Depthbuffer(unsigned int w, unsigned int h);

        virtual ~Depthbuffer();

        virtual void clear();

        inline float getDepth(unsigned int x, unsigned int y) const
        {
            return data[x + width * y];
        }

        inline void plot(unsigned int x, unsigned int y, float z)
        {
            data[x + width * y] = z;
        }

        bool conditionalPlot(const glm::vec3 &pos);

        bool conditionalPlot(int x, int y, float z);


    protected:
        unsigned int width, height;
        float *data;
    };

}

#endif