#include "CubeGeometry.h"

#include "../rendering/Pipeline.h"

#include <random>

using glm::vec4;
using render::Vertex;

namespace geometry
{

    CubeGeometry::CubeGeometry(const glm::vec3 &sidelength)
    {
        // create a cube
        vertices.push_back(Vertex(vec4(-1, 1, 1, 1)));
        vertices.push_back(Vertex(vec4(-1, -1, 1, 1)));
        vertices.push_back(Vertex(vec4(1, -1, 1, 1)));
        vertices.push_back(Vertex(vec4(1, 1, 1, 1)));
        vertices.push_back(Vertex(vec4(-1, 1, -1, 1)));
        vertices.push_back(Vertex(vec4(-1, -1, -1, 1)));
        vertices.push_back(Vertex(vec4(1, -1, -1, 1)));
        vertices.push_back(Vertex(vec4(1, 1, -1, 1)));

        vec4 halfSide = vec4(sidelength * 0.5f, 1.0f);

        for (int i = 0; i < 8; ++i) {
            vertices[i].position *= halfSide;
        }


        for (int i = 0; i < 8; ++i) {
            float r = (float) std::rand() / RAND_MAX;
            float g = (float) std::rand() / RAND_MAX;
            float b = 1.f - (r + g);
            vertices[i].color.r = r;
            vertices[i].color.g = g;
            vertices[i].color.b = b;
        }

        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(2);
        indices.push_back(3);
        indices.push_back(3);
        indices.push_back(0);
        indices.push_back(4);
        indices.push_back(5);
        indices.push_back(5);
        indices.push_back(6);
        indices.push_back(6);
        indices.push_back(7);
        indices.push_back(7);
        indices.push_back(4);
        indices.push_back(0);
        indices.push_back(4);
        indices.push_back(1);
        indices.push_back(5);
        indices.push_back(2);
        indices.push_back(6);
        indices.push_back(3);
        indices.push_back(7);
    }
}