#ifndef VIEWPORT_INCLUDED
#define VIEWPORT_INCLUDED

#include <glm/glm.hpp>

struct Line2D;

class Viewport
{
public:
	Viewport(unsigned int x, unsigned int y, unsigned int width, unsigned int height);


	bool isInside(const glm::ivec2& p) const;
	bool isInside(const Line2D& line) const;

	glm::vec3 calculateWindowCoordinates(const glm::vec3& ndc) const;

	glm::ivec2		origin;
	glm::ivec2		size;
};

#endif