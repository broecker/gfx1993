#ifndef SHADING_GEOMETRY_INCLUDED
#define SHADING_GEOMETRY_INCLUDED

#include <glm/glm.hpp>

struct ShadingGeometry
{
	glm::vec3	worldPosition;
	glm::vec3	normal;

	glm::vec4	colour;

	glm::ivec2	windowCoord;

};

ShadingGeometry interpolate(const ShadingGeometry& a, const ShadingGeometry& b, float d);

#endif