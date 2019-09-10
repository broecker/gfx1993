#include "CubeGeometry.h"

using glm::vec4;

namespace geo {

CubeGeometry::CubeGeometry(const glm::vec3& sidelength)
{
	// create a cube
	vertices.push_back( Vertex( vec4(-1,  1, 1, 1) ) );
	vertices.push_back( Vertex( vec4(-1, -1, 1, 1) ) );
	vertices.push_back( Vertex( vec4( 1, -1, 1, 1) ) );
	vertices.push_back( Vertex( vec4( 1,  1, 1, 1) ) );
	vertices.push_back( Vertex( vec4(-1,  1, -1, 1) ) );
	vertices.push_back( Vertex( vec4(-1, -1, -1, 1) ) );
	vertices.push_back( Vertex( vec4( 1, -1, -1, 1) ) );
	vertices.push_back( Vertex( vec4( 1,  1, -1, 1) ) );

	vec4 halfSide = vec4(sidelength / 2.0f, 1.0f);

	for (int i = 0; i < 8; ++i) 
	{
		vertices[i].position *= halfSide;
	}


	for (int i = 0; i < 8; ++i)
	{
		float r = (float)rand() / RAND_MAX;
		float g = (float)rand() / RAND_MAX;
		float b = 1.f - (r+g);
		vertices[i].colour.r = r; 
		vertices[i].colour.g = g; 
		vertices[i].colour.b = b; 
	}

	indices.push_back(0); indices.push_back(1);
	indices.push_back(1); indices.push_back(2);
	indices.push_back(2); indices.push_back(3);
	indices.push_back(3); indices.push_back(0);
	indices.push_back(4); indices.push_back(5);
	indices.push_back(5); indices.push_back(6);
	indices.push_back(6); indices.push_back(7);
	indices.push_back(7); indices.push_back(4);
	indices.push_back(0); indices.push_back(4);
	indices.push_back(1); indices.push_back(5);
	indices.push_back(2); indices.push_back(6);
	indices.push_back(3); indices.push_back(7);
}

const VertexList& CubeGeometry::getVertices() const
{
	return vertices;
}

const IndexList& CubeGeometry::getIndices() const
{
	return indices;
}

}