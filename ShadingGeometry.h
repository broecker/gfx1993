#ifndef SHADING_GEOMETRY_INCLUDED
#define SHADING_GEOMETRY_INCLUDED

#include <glm/glm.hpp>

struct ShadingGeometry
{
	glm::vec4	worldPosition;
	glm::vec3	normal;

	glm::vec4	colour;
};

ShadingGeometry interpolate(const ShadingGeometry& a, const ShadingGeometry& b, float d);

#endif