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

	// loads a PLY geometry
	bool loadPLY(const std::string& filename);

	// centers the model's vertices around its origin
	void center();

};

#endif