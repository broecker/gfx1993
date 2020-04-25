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

        inline unsigned int getWidth() const { return width; }
        inline unsigned int getHeight() const { return height; }

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

        inline bool isVisible(int x, int y, float z) const
        {
            return z < data[x + width*y];
        }



    protected:
        unsigned int width, height;
        float *data;
    };

}

#endif