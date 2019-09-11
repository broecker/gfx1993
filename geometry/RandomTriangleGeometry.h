#ifndef RANDOM_TRIANGLE_GEOMETRY_INCLUDED
#define RANDOM_TRIANGLE_GEOMETRY_INCLUDED

#include "Geometry.h"

namespace geo {

class RandomTriangleGeometry : public Geometry
{
public:
	RandomTriangleGeometry(const glm::vec3& boundsMin, const glm::vec3& boundsMax);

	// Clears all contained triangles.
	void clear();

	// Adds a double sided triangle.
	void addTriangle();

private:
	glm::vec3		boundsMin, boundsMax;

	Vertex&& createRandomVertex() const;
	void addTriangle(const Vertex& a, const Vertex& b, const Vertex& c);
};

}

#endif
