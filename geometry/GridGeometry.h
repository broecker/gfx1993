#ifndef GRID_GEOMETRY_INCLUDED
#define GRID_GEOMETRY_INCLUDED

#include "Geometry2.h"

namespace geo {

// A 2D grid on the XZ plane.
class GridGeometry : public Geometry2 
{
public:
	GridGeometry();

	const VertexList& getVertices() const override;
	const IndexList& getIndices() const override;

private:
	VertexList	vertices;
	IndexList	indices;
};

}

#endif
