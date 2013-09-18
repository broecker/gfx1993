#ifndef GEOMETRY_INCLUDED
#define GEOMETRY_INCLUDED

#include "Vertex.h"

#include <glm/glm.hpp>
#include <string>

struct Geometry
{
	glm::mat4	modelMatrix;

	float 		boundingSphereRadius;

	VertexList	vertices;
	IndexList	indices;

	bool loadPLY(const std::string& filename);
};

#endif