#include "RandomTriangleGeometry.h"

#include <glm/gtc/random.hpp> 

using glm::vec3;

namespace geo {

RandomTriangleGeometry::RandomTriangleGeometry(const vec3& min, const vec3& max) : boundsMin(min), boundsMax(max)
{
}

void RandomTriangleGeometry::addTriangle()
{
	Vertex a = createRandomVertex();
	Vertex b = createRandomVertex();
	Vertex c = createRandomVertex();

	addTriangle(a, b, c);
	addTriangle(a, c, b);
}

void RandomTriangleGeometry::addTriangle(const Vertex& a, const Vertex& b, const Vertex& c)
{
	vertices.push_back(a);
	indices.push_back( vertices.size()-1 );

	vertices.push_back(b);
	indices.push_back( vertices.size()-1 );

	vertices.push_back(c);
	indices.push_back( vertices.size()-1 );

	// Calculate normal for the triangle.
	vec3 u = glm::normalize(b.position - a.position);
	vec3 v = glm::normalize(c.position - a.position);
	vec3 n = glm::normalize(glm::cross(u, v));

	vertices[vertices.size()-3].normal = n;
	vertices[vertices.size()-2].normal = n;
	vertices[vertices.size()-1].normal = n;
}

void RandomTriangleGeometry::clear()
{
	indices.clear();
	vertices.clear();
}

const IndexList& RandomTriangleGeometry::getIndices() const
{
	return indices;
}

const VertexList& RandomTriangleGeometry::getVertices() const
{
	return vertices;
}

Vertex&& RandomTriangleGeometry::createRandomVertex() const
{
	vec3 pos = glm::linearRand(boundsMin, boundsMax);
	vec3 normal = glm::ballRand(1.f);

	glm::vec2 texcoord(0,0);

	float r = (float)rand() / RAND_MAX;
	float g = (float)rand() / RAND_MAX;
	float b = 1.f - r - g;

	return std::move(Vertex(glm::vec4(pos, 1.f), normal, glm::vec4(r,g,b,1.f), texcoord));		
}

}