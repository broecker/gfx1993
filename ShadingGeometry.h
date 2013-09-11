#ifndef SHADING_GEOMETRY_INCLUDED
#define SHADING_GEOMETRY_INCLUDED

#include <glm/glm.hpp>

struct ShadingGeometry
{
	// world position and normal
	glm::vec3	position;
	glm::vec3	normal;

	glm::vec4	colour;
	glm::vec2	texcoord;

	glm::ivec2	windowCoord;
	float		depth;

};

ShadingGeometry interpolate(const ShadingGeometry& a, const ShadingGeometry& b, float d);

#endif