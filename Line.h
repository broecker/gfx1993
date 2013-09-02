#ifndef LINE_INCLUDED
#define LINE_INCLUDED

#include <glm/glm.hpp>

#include "Vertex.h"

struct Line2D
{
	// two end positions
	glm::vec2	a, b;

	Line2D(const glm::vec2& a, const glm::vec2& b);

	// calculates the intersection point for these two lines
	glm::vec2 intersect(const Line2D& other) const;


};

#endif