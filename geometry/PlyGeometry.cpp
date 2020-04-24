#include "PlyGeometry.h"
#include "../rendering/Pipeline.h"

#include <fstream>
#include <algorithm>
#include <cstdio>
#include <cassert>
#include <iostream>

using render::Vertex;

namespace geometry
{

    PlyGeometry::PlyGeometry() : boundingSphereRadius(0)
    {
    }

    bool PlyGeometry::loadPly(const std::string &filename)
    {
        boundingSphereRadius = 0.f;

        std::ifstream file(filename.c_str());
        if (!file.is_open()) {
            std::cerr << "Unable to open file \"" << filename << "\"\n";
            return false;
        } else
            std::clog << "Loading file " << filename << std::endl;

        std::string buffer;

        bool readHeader = true;
        unsigned int vertexCount = 0, readVertices = 0;
        unsigned int faceCount = 0;
        while (!file.eof()) {
            std::getline(file, buffer);

            if (buffer.empty())
                continue;

            if (readHeader) {
                if (buffer == "end_header")
                    readHeader = false;

                if (buffer.find("element vertex") != std::string::npos) {
                    assert(sscanf(buffer.c_str(), "element vertex %i", &vertexCount) == 1);
                    vertices.reserve(vertexCount);
                }

                if (buffer.find("element face") != std::string::npos) {
                    assert(sscanf(buffer.c_str(), "element face %i", &faceCount) == 1);
                    indices.reserve(faceCount * 3);
                }

                continue;
            }

            // read number of vertices
            if (readVertices < vertexCount) {
                float x, y, z, c, i;
                assert(sscanf(buffer.c_str(), "%f %f %f %f %f", &x, &y, &z, &c, &i) == 5);

                vertices.push_back(Vertex(glm::vec4(x, y, z, 1)));

                boundingSphereRadius = std::max(boundingSphereRadius,
                                                sqrtf(x * x + y * y + z * z));

                ++readVertices;
            }

                // and then the faces
            else {
                unsigned int a, b, c;
                assert(sscanf(buffer.c_str(), "3 %i %i %i", &a, &b, &c) == 3);

                indices.push_back(a);
                indices.push_back(b);
                indices.push_back(c);

                if (a >= vertices.size())
                    std::cerr << "Illegal index: " << a << "?\n";

                if (b >= vertices.size())
                    std::cerr << "Illegal index: " << b << "?\n";

                if (c >= vertices.size())
                    std::cerr << "Illegal index: " << c << "?\n";
            }
        }

        std::clog << "Read " << vertices.size() << " vertices, " << indices.size() << " indices" << std::endl;

        // calculate normals
        for (size_t i = 0; i < indices.size(); i += 3) {

            // triangle
            Vertex &a = vertices[indices[i + 0]];
            Vertex &b = vertices[indices[i + 1]];
            Vertex &c = vertices[indices[i + 2]];

            // normal
            glm::vec3 n = glm::normalize(glm::cross(glm::vec3(b.position - a.position),
                                                    glm::vec3(c.position - a.position)));

            a.normal += n;
            b.normal += n;
            c.normal += n;
        }

        std::clog << "Renormalizing " << vertices.size() << " vertex normals." << std::endl;
        // scale normal to length 1;
        for (auto v : vertices) {
            v.normal = glm::normalize(v.normal);
        }

        return true;
    }

    void PlyGeometry::center()
    {
        // find bounding box
        glm::vec3 min(FLT_MAX), max(FLT_MIN);

        for (auto v : vertices) {
            min.x = std::min(min.x, v.position.x);
            min.y = std::min(min.y, v.position.y);
            min.z = std::min(min.z, v.position.z);

            max.x = std::max(max.x, v.position.x);
            max.y = std::max(max.y, v.position.y);
            max.z = std::max(max.z, v.position.z);
        }

        // update vertices and find new bounding sphere radius
        boundingSphereRadius = 0.f;
        for (auto v : vertices) {
            v.position = v.position - glm::vec4(min, 0.f) - glm::vec4((max - min) * 0.5f, 0.f);
            boundingSphereRadius = std::max(boundingSphereRadius, glm::length(glm::vec3(v.position)));
        }

    }
}