#include "GridGeometry.h"

using glm::vec3;
using glm::vec4;

namespace geo {

GridGeometry::GridGeometry()
{
	// Create a grid
	for (int x = -10; x <= 10; ++x)
	{
		vertices.push_back(Vertex(vec4(x, 0, -10, 1)));
		vertices.push_back(Vertex(vec4(x, 0,  10, 1)));

		indices.push_back(vertices.size()-1);
		indices.push_back(vertices.size()-2);
	}

	for (int z = -10; z <= 10; ++z) 
	{
		vertices.push_back(Vertex(vec4(-10, 0, z, 1)));
		vertices.push_back(Vertex(vec4( 10, 0, z, 1)));

		indices.push_back(vertices.size()-1);
		indices.push_back(vertices.size()-2);
	}

	for (auto v = vertices.begin(); v != vertices.end(); ++v)
	{
		v->normal = vec3(0, 1, 0);
		v->colour = vec4(1, 1, 1, 1);
	}
}

const VertexList& GridGeometry::getVertices() const
{
	return vertices;
}

const IndexList& GridGeometry::getIndices() const
{
	return indices;
}

}