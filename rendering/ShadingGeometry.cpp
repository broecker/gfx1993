#include "ShadingGeometry.h"

namespace render 
{

ShadingGeometry interpolate(const ShadingGeometry& a, const ShadingGeometry& b, float d)
{
	using namespace glm;

	ShadingGeometry result;

	result.position = mix(a.position, b.position, d);
	result.normal = normalize(mix(a.normal, b.normal, d));

	result.colour = mix(a.colour, b.colour, d);
	result.windowCoord = mix(a.windowCoord, b.windowCoord, d);

	return result;
}

}