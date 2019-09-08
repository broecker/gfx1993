#ifndef RANDOM_TRIANGLE_GEOMETRY_INCLUDED
#define RANDOM_TRIANGLE_GEOMETRY_INCLUDED

#include "Geometry2.h"

class RandomTriangleGeometry : public Geometry2
{
public:
	RandomTriangleGeometry(const glm::vec3& boundsMin, const glm::vec3& boundsMax);

	const IndexList& getIndices() const override;
	const VertexList& getVertices() const override;

	// Clears all contained triangles.
	void clear();

	// Adds a double sided triangle.
	void addTriangle();

private:
	glm::vec3		boundsMin, boundsMax;

	VertexList		vertices;
	IndexList		indices;

	Vertex&& createRandomVertex() const;
	void addTriangle(const Vertex& a, const Vertex& b, const Vertex& c);
};



#endif
