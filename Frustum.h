#ifndef FRUSTUM_INCLUDED
#define FRUSTUM_INCLUDED

#include "Plane.h"
#include <glm/glm.hpp>

struct Frustum
{
	Plane	top, bottom, left, right, near, far;

	void calculate(const glm::mat4& mat);
};
#endif