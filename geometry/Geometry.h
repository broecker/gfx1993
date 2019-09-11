#ifndef GEOMETRY2_INCLUDED
#define GEOMETRY2_INCLUDED

#include "../common/Vertex.h"

namespace geo {

class Geometry
{
public:
	Geometry();
	virtual ~Geometry() = default;

	inline const VertexList& getVertices() const { return vertices; }
	inline const IndexList& getIndices() const  { return indices; }

	// Access to transform is public -- no reason to write getter+setter
	// for the most-used member.	
	glm::mat4 transform;

protected:
    // Child classes should write to these two members.
    VertexList  vertices;
    IndexList   indices;
};

}

#endif
