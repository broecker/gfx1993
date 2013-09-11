#include "RenderPrimitive.h"


ClipResult PointPrimitive::clipToNDC() const
{
	if (p.clipPosition.x >= -p.clipPosition.w && p.clipPosition.x <= p.clipPosition.w && 
		p.clipPosition.y >= -p.clipPosition.w && p.clipPosition.y <= p.clipPosition.w &&
		p.clipPosition.z >= -p.clipPosition.w && p.clipPosition.z <= p.clipPosition.w)
		return KEEP;
	else
		return DISCARD;
}

ShadingGeometry PointPrimitive::interpolate(float d) const
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

}