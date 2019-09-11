#ifndef GEOMETRY2_INCLUDED
#define GEOMETRY2_INCLUDED

#include "../common/Vertex.h"

namespace geo {

class Geometry
{
public:
	Geometry();
	virtual ~Geometry() = default;

	virtual const VertexList& getVertices() const = 0;
	virtual const IndexList& getIndices() const = 0;

	// Access to transform is public -- no reason to write getter+setter
	// for the most-used member.	
	glm::mat4 transform;
};

}

#endif
