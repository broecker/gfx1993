#ifndef RENDER_PRIMITIVE_INCLUDED
#define RENDER_PRIMITIVE_INCLUDED

#include <vector>
#include "Vertex.h"
#include "ShadingGeometry.h"

struct LinePrimitive
{
	VertexOut	a, b;

	inline LinePrimitive(const VertexOut& a_, const VertexOut& b_) : a(a_), b(b_) {};

	VertexOut interpolate(float d) const;


};

typedef std::vector<LinePrimitive> LinePrimitiveList;

#endif