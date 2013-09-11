#ifndef RENDER_PRIMITIVE_INCLUDED
#define RENDER_PRIMITIVE_INCLUDED

#include <vector>
#include "Vertex.h"
#include "ShadingGeometry.h"

enum ClipResult
{
	KEEP,
	DISCARD,
	CLIPPED
};

struct PointPrimitive
{
	VertexOut	p;
	inline PointPrimitive(const VertexOut& o) : p(o) {}

	ShadingGeometry interpolate(float d) const;

	ClipResult clipToNDC() const;

};

struct LinePrimitive
{
	VertexOut	a, b;

	inline LinePrimitive(const VertexOut& a_, const VertexOut& b_) : a(a_), b(b_) {};

	ShadingGeometry interpolate(float d) const;

	ClipResult clipToNDC();
};

typedef std::vector<PointPrimitive> PointPrimitiveList;
typedef std::vector<LinePrimitive> LinePrimitiveList;

#endif