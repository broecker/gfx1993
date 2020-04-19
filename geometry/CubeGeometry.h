#ifndef CUBE_GEOMETRY_INCLUDED
#define CUBE_GEOMETRY_INCLUDED

#include "Geometry.h"

namespace geo
{

    class CubeGeometry : public Geometry
    {
    public:
        explicit CubeGeometry(const glm::vec3 &sideLength);
    };

}

#endif
