#ifndef FRUSTUM_INCLUDED
#define FRUSTUM_INCLUDED

#include "Plane.h"
#include <glm/glm.hpp>

struct Frustum
{
	enum PlaneName
	{
		NEAR,
		FAR,
		TOP,
		BOTTOM,
		LEFT,
		RIGHT
	};
	
	Plane	plane[6];

	void calculate(const glm::mat4& mat);
};
#endif