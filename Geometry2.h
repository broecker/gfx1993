#ifndef GEOMETRY2_INCLUDED
#define GEOMETRY2_INCLUDED

#include "Vertex.h"

#include <glm/glm.hpp>

class Geometry2
{
public:
	virtual ~Geometry2();

	virtual const VertexList& getVertices() const = 0;
	virtual const IndexList& getIndices() const = 0;

	// Access to transform is public -- no reason to write getter+setter
	// for the most-used member.	
	glm::mat4 transform;
};

#endif
