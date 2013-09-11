#include "ShadingGeometry.h"


ShadingGeometry interpolate(const ShadingGeometry& a, const ShadingGeometry& b, float d)
{
	using namespace glm;

	ShadingGeometry result;

	result.worldPosition = mix(a.worldPosition, b.worldPosition, d);
	result.normal = normalize(mix(a.normal, b.normal, d));

	result.colour = mix(a.colour, b.colour, d);


	return result;
}