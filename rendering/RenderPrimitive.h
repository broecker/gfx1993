#ifndef RENDER_PRIMITIVE_INCLUDED
#define RENDER_PRIMITIVE_INCLUDED

#include <vector>
#include "../common/Vertex.h"
#include "ShadingGeometry.h"

namespace render 
{

enum ClipResult
{
	KEEP,
	DISCARD,
	CLIPPED
};

struct PointPrimitive
{
	VertexOut	p;
	inline explicit PointPrimitive(const VertexOut& o) : p(o) {}

	ShadingGeometry rasterise() const;

	ClipResult clipToNDC() const;
};

struct LinePrimitive
{
	VertexOut	a, b;

	inline LinePrimitive(const VertexOut& a_, const VertexOut& b_) : a(a_), b(b_) {};

	ShadingGeometry rasterise(float d) const;


	ClipResult clipToNDC();
};

struct TrianglePrimitive
{
	VertexOut	a, b, c;


	inline TrianglePrimitive(const VertexOut& a_, const VertexOut& b_, const VertexOut& c_) : a(a_), b(b_), c(c_) {}
	ShadingGeometry rasterise(const glm::vec3& bary) const;

	ClipResult clipToNDC();


};

typedef std::vector<PointPrimitive> PointPrimitiveList;
typedef std::vector<LinePrimitive> LinePrimitiveList;
typedef std::vector<TrianglePrimitive> TrianglePrimitiveList;

}
#endif