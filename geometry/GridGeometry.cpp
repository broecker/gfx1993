#include "GridGeometry.h"
#include "../rendering/Pipeline.h"

#include <glm/glm.hpp>

using glm::vec3;
using glm::vec4;
using render::Vertex;

namespace geo
{

    GridGeometry::GridGeometry()
    {
        static const int LENGTH = 100;
        static const int STEP = 5;

        static const vec4 MAJOR_COLOR(1.f);
        static const vec4 MINOR_COLOR(0, 0, 0.7f, 1);

        // Create a grid
        for (int x = -LENGTH; x <= LENGTH; x += STEP) {
            Vertex a(vec4(x, 0, -LENGTH, 1));
            Vertex b(vec4(x, 0, LENGTH, 1));

            vec4 color = MINOR_COLOR;
            if (x % (STEP * 10) == 0) {
                color = MAJOR_COLOR;
            }
            a.color = color;
            b.color = color;

            vertices.push_back(a);
            vertices.push_back(b);
            indices.push_back(vertices.size() - 1);
            indices.push_back(vertices.size() - 2);
        }

        for (int z = -LENGTH; z <= LENGTH; z += STEP) {
            Vertex a(vec4(-LENGTH, 0, z, 1));
            Vertex b(Vertex(vec4(LENGTH, 0, z, 1)));

            vec4 color = MINOR_COLOR;
            if (z % (STEP * 10) == 0) {
                color = MAJOR_COLOR;
            }
            a.color = color;
            b.color = color;

            vertices.push_back(a);
            vertices.push_back(b);
            indices.push_back(vertices.size() - 1);
            indices.push_back(vertices.size() - 2);
        }

        for (auto v : vertices) {
            v.normal = vec3(0, 1, 0);
            v.color = vec4(1, 1, 1, 1);
        }
    }
}