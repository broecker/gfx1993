#include "RenderPrimitive.h"

using namespace glm;


ClipResult PointPrimitive::clipToNDC() const
{
	if (p.clipPosition.x >= -p.clipPosition.w && p.clipPosition.x <= p.clipPosition.w && 
		p.clipPosition.y >= -p.clipPosition.w && p.clipPosition.y <= p.clipPosition.w &&
		p.clipPosition.z >= -p.clipPosition.w && p.clipPosition.z <= p.clipPosition.w)
		return KEEP;
	else
		return DISCARD;
}

ShadingGeometry PointPrimitive::rasterise() const
{
	ShadingGeometry result;
	result.position = p.worldPosition;
	result.normal = p.worldNormal;
	result.colour = p.colour;
	result.texcoord = p.texcoord;

	return result;
}

ClipResult LinePrimitive::clipToNDC()
{
	return KEEP;
}

ShadingGeometry LinePrimitive::rasterise(float d) const
{
	ShadingGeometry result;
	result.position = mix(a.worldPosition, b.worldPosition, d);
	result.normal = normalize(mix(a.worldNormal, b.worldNormal, d));
	result.colour = mix(a.colour, b.colour, d);
	result.texcoord = mix(a.texcoord, b.texcoord, d);

	return result;
}
