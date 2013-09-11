#include "Line.h"

Line2D::Line2D(const glm::vec2& A, const glm::vec2& B) : a(A), b(B)
{
}


Vertex Line3D::interpolate(float d) const
{
	return Vertex(	glm::mix(a.position, b.position, d),
					glm::normalize(glm::mix(a.normal, b.normal, d)),
					glm::mix(a.colour, b.colour, d) );
	
}