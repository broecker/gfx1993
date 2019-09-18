#ifndef GFX1993_CLIPPER_H
#define GFX1993_CLIPPER_H

#include "Pipeline.h"

#include <vector>
#include <glm/glm.hpp>

namespace render
{
    class Clipper
    {
    public:
        struct Plane {
            glm::vec3   normal;
            float       d;

            // Create a plane. Make sure that plane.w is the *negative* distance from the Origin.
            explicit Plane(const glm::vec4& plane) : normal(glm::normalize(glm::vec3(plane.x, plane.y, plane.z))), d(-plane.w) {}

            inline bool inFrontSpace(const glm::vec3& point) const {
                return distance(point) >= 0;
            }

            inline float distance(const glm::vec3& point) const {
                return glm::dot(point, normal) - d;
            }
        };

        Clipper(const Plane& plane);
        Clipper(const std::vector<Plane>& planes);
        Clipper();

        PointPrimitiveList clipPoints(const PointPrimitiveList& points) const;

        LinePrimitiveList clipLines(const LinePrimitiveList& lines) const;

    private:
        std::vector<Plane>      planes;
    };

}

#endif //GFX1993_CLIPPER_H
