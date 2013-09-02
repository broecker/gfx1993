#include "Viewport.h"

#include "Line.h"

Viewport::Viewport(unsigned int x, unsigned int y, unsigned int w, unsigned int h) : 
	origin(x, y), size(w, h)
{
}

glm::vec3 Viewport::calculateWindowCoordinates(const glm::vec3& ndc) const
{
	const float rangeFar = 1.f;
	const float rangeNear = 0.f;

	glm::vec2 pos = glm::vec2(ndc.x, ndc.y) * glm::vec2(size)/2.f  + glm::vec2(origin + size/2);
	float z = ((rangeFar-rangeNear)/2.f) * ndc.z + (rangeFar+rangeNear)/2.f;

	return glm::vec3(pos.x, pos.y, z);
}

bool Viewport::isInside(const glm::ivec2& p) const
{
	glm::ivec2 t = p - origin;
	return (t.x <= size.x && t.y <= size.y);
}

bool Viewport::isInside(const Line2D& line) const
{
	glm::ivec2 a = glm::ivec2(line.a) - origin;
	glm::ivec2 b = glm::ivec2(line.b) - origin;

	// test if both line ends are on the same side of the box
	if (a.x < 0 && b.x < 0)
		return false;
	if (a.x > size.x && b.x > size.x)
		return false;
	if (a.y < 0 && b.y < 0)
		return false;
	if (a.y > size.y && b.y > size.y)
		return false;

	/* 	This still gives a wrong positive result, eg when one vertex is on the 
		top the other on the left of the box. This case should be dealt with 
		clipping.
	*/
		
	return true;

}