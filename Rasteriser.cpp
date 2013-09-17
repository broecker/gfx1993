#include "Rasteriser.h"
#include "Framebuffer.h"
#include "Depthbuffer.h"
#include "Viewport.h"

#include "Shader.h"
#include "glmHelper.h"
#include "Line.h"
#include "Frustum.h"
#include "ShadingGeometry.h"
#include "RenderPrimitive.h"

#include <algorithm>
#include <iostream>
#include <list>
#include <boost/function.hpp>


// a helper struct/functor object for STL algorithms
struct CallVertexShader
{
	VertexShader*	shader;
	inline CallVertexShader(VertexShader* sh) : shader(sh) {} 

	inline VertexOut operator () (const Vertex& in)
	{
		return shader->transformSingle( in );
	};
};

// noop vertex transform -- used when there is no vertex shader
struct NoVertexTransform
{
	inline VertexOut operator () (const Vertex& in)
	{
		VertexOut result;
		result.clipPosition = in.position;
		result.worldPosition = glm::vec3(in.position);
		result.worldNormal = in.normal;
		result.colour = in.colour;
		result.texcoord = in.texcoord;
		return result;
	}
};

unsigned int Rasteriser::drawPoints(const VertexList& vertices) const
{
	using namespace glm;

	if (!fragmentShader)
		throw "Fragment shader missing!";

	if (!framebuffer)
		throw "Framebuffer missing!";

	if (!depthbuffer)
		throw "Depth buffer missing!";

	if (!viewport)
		throw "Viewport is missing!";

	unsigned int pointsDrawn = 0;

	VertexOutList transformedVertices(vertices.size());
	transformVertices(vertices, transformedVertices);


	// build points
	PointPrimitiveList points;
	for (VertexOutList::const_iterator po = transformedVertices.begin(); po != transformedVertices.end(); ++po)
	{
		PointPrimitive pt(*po);

		// check clipping/culling here
		if (pt.clipToNDC() == DISCARD)
			continue;

		points.push_back( pt );
	}

	for (PointPrimitiveList::const_iterator it = points.begin(); it != points.end(); ++it)
	{
		const PointPrimitive& p = *it;

		// perspective divide and window coordinates
		const vec4& pos_clip = p.p.clipPosition;
		const vec3 pos_ndc = vec3(pos_clip) / pos_clip.w;
		const vec3 pos_win = viewport->calculateWindowCoordinates( pos_ndc );

		// if there is a depth buffer but the depth test fails -> discard fragment (early) 
		if (depthbuffer && !depthbuffer->conditionalPlot(pos_win))
			continue;


		// calculate shading geometry
		ShadingGeometry sgeo = p.rasterise();
		sgeo.windowCoord = ivec2(pos_win);
		sgeo.depth = pos_win.z;

		// shade fragment and plot
		Colour fragColour = fragmentShader->shadeSingle( sgeo );
		framebuffer->plot(sgeo.windowCoord, fragColour);

		++pointsDrawn;
	}

	return pointsDrawn;
}

void Rasteriser::transformVertices(const VertexList& vertices, VertexOutList& out) const
{
	if (vertexShader)
	{
		CallVertexShader vertexTransform(vertexShader);
		std::transform(vertices.begin(), vertices.end(), out.begin(), vertexTransform);
	}
	else
	{
		NoVertexTransform vertexTransform;
		std::transform(vertices.begin(), vertices.end(), out.begin(), vertexTransform);
	}
}

unsigned int Rasteriser::drawLines(const VertexList& vertices, const IndexList& indices) const
{
	unsigned int linesDrawn = 0;

	using namespace glm;
	
	if (!fragmentShader)
		throw "Fragment shader missing!";

	if (!viewport)
		throw "Viewport is missing!";
			

	// transform vertices
	VertexOutList transformedVertices( vertices.size() );
	transformVertices(vertices, transformedVertices);

	// build lines
	LinePrimitiveList lines;
	for (int i = 0; i < indices.size(); i += 2)
	{
		const VertexOut& a = transformedVertices[ indices[i+0] ];
		const VertexOut& b = transformedVertices[ indices[i+1] ];
		lines.push_back( LinePrimitive(a, b) );
	
	}
	
	for(LinePrimitiveList::iterator l = lines.begin(); l != lines.end(); ++l)
	{
		// clip and cull lines here
		ClipResult cr = l->clipToNDC();
		if (cr == DISCARD)
		{
			// line completely outside -- discard it
			continue;
		}

		drawLine( *l );
		++linesDrawn;
		
	}

	return linesDrawn;
}

unsigned int Rasteriser::drawTriangles(const VertexList& vertices, const IndexList& indices) const
{
	unsigned int trianglesDrawn = 0;

	using namespace glm;
	
	if (!vertexShader)
		throw "Vertex shader missing!";
	
	if (!fragmentShader)
		throw "Fragment shader missing!";

	if (!viewport)
		throw "Viewport is missing!";
			
	// transform vertices
	VertexOutList transformedVertices( vertices.size() );
	transformVertices( vertices, transformedVertices );

	// build triangles
	TrianglePrimitiveList triangles;
	for (int i = 0; i < indices.size(); i += 3)
	{
		const VertexOut& a = transformedVertices[ indices[i+0] ];
		const VertexOut& b = transformedVertices[ indices[i+1] ];
		const VertexOut& c = transformedVertices[ indices[i+2] ];


		// backface culling
		vec3 n = cross(vec3(b.clipPosition-a.clipPosition), vec3(c.clipPosition-a.clipPosition));
		if (n.z >= 0)
			triangles.push_back( TrianglePrimitive(a, b, c) );

	}

	for(TrianglePrimitiveList::iterator tri = triangles.begin(); tri != triangles.end(); ++tri)
	{
		// clip and cull triangles here
		ClipResult cr = tri->clipToNDC();
		if (cr == DISCARD)
		{
			// triangle completely outside -- discard it
			continue;
		}

		if (cr == CLIPPED)
		{
			// insert newly created triangles at the end

		}

		// rasterise and interpolate triangle here
		drawTriangle( *tri );

	}




	return trianglesDrawn;
}

// bresenham line drawing
void Rasteriser::drawLine(const LinePrimitive& line) const
{
	using namespace glm;

	// point a
	const vec4& posA_clip = line.a.clipPosition;
	const vec3 posA_ndc = vec3(posA_clip) / posA_clip.w;
	const vec3 posA_win = viewport->calculateWindowCoordinates( posA_ndc );

	// point b
	const vec4& posB_clip = line.b.clipPosition;
	const vec3 posB_ndc = vec3(posB_clip) / posB_clip.w;
	const vec3 posB_win = viewport->calculateWindowCoordinates( posB_ndc );

	ivec2 a = ivec2(posA_win);
	ivec2 b = ivec2(posB_win);

	float lineLength = length(b-a); 
	unsigned int positionCounter = 0;

	int dx = abs(a.x-b.x), sx = a.x<b.x ? 1 : -1;
	int dy = abs(a.y-b.y), sy = a.y<b.y ? 1 : -1; 
	int err = (dx>dy ? dx : -dy)/2, e2;

	for(;;)
	{
		// interpolation for depth and texturing/colour
		float delta = std::min(1.f, positionCounter/lineLength);
		float depth = mix(posA_win.z, posB_win.z, delta);

		if (viewport->isInside(a) && (!depthbuffer || (depthbuffer && depthbuffer->conditionalPlot(a.x, a.y, depth))))
		{			
			ShadingGeometry sgeo = line.rasterise( positionCounter/lineLength );
			sgeo.windowCoord = a;
			sgeo.depth = depth;

			Colour c = fragmentShader->shadeSingle(sgeo);
			framebuffer->plot(a, c);
		}

		// 'core' bresenham
		if (a.x==b.x && a.y==b.y) 
			break;
		e2 = err;
		if (e2 >-dx) { err -= dy; a.x += sx; }
		if (e2 < dy) { err += dx; a.y += sy; }

		++positionCounter;
	}
}

// finds which part of the half-space of line a-b point c is in (positive or negative)
static inline int pointInHalfspace(const glm::ivec2& a, const glm::ivec2& b, const glm::ivec2& c)
{
	return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
}

// parameter-based rasterisation of triangles
void Rasteriser::drawTriangle(const TrianglePrimitive& triangle) const
{
	using namespace glm;
	// point a
	const vec4& posA_clip = triangle.a.clipPosition;
	const vec3 posA_ndc = vec3(posA_clip) / posA_clip.w;
	const vec3 posA_win = viewport->calculateWindowCoordinates( posA_ndc );

	// point b
	const vec4& posB_clip = triangle.b.clipPosition;
	const vec3 posB_ndc = vec3(posB_clip) / posB_clip.w;
	const vec3 posB_win = viewport->calculateWindowCoordinates( posB_ndc );

	// point c
	const vec4& posC_clip = triangle.c.clipPosition;
	const vec3 posC_ndc = vec3(posC_clip) / posC_clip.w;
	const vec3 posC_win = viewport->calculateWindowCoordinates( posC_ndc );

	// three window/screen coordinates  
	ivec2 a = ivec2(posA_win);
	ivec2 b = ivec2(posB_win);
	ivec2 c = ivec2(posC_win);

	// calculate bounds
	ivec2 min, max;
	min.x = glm::min(a.x, glm::min(b.x, c.x));
	min.y = glm::min(a.y, glm::min(b.y, c.y));
	max.x = glm::max(a.x, glm::max(b.x, c.x));
	max.y = glm::max(a.y, glm::max(b.y, c.y));

	// clip against screen coords
	min = glm::max(viewport->origin, min);
	max = glm::min(viewport->origin+viewport->size-1, max);

	/*
	// draw bounding box	
	Colour bboxColour(1,0,0,1);
	for (unsigned int x = min.x; x <= max.x; ++x)
	{
		ivec2 p(x, min.y);
		framebuffer->plot(p, bboxColour);
		p = ivec2(x, max.y);
		framebuffer->plot(p, bboxColour);
	}

	for (unsigned int y = min.y; y <= max.y; ++y)
	{
		ivec2 p(min.x, y);
		framebuffer->plot(p, bboxColour);
		p = ivec2(max.x, y);
		framebuffer->plot(p, bboxColour);
	}
	*/
	
	// rasterise
	// loop over the bounding box
	for (unsigned int y = min.y; y <= max.y; ++y)
	{
		for (unsigned int x = min.x; x <= max.x; ++x)
		{
			// position
			ivec2 p(x,y);

			int w0 = pointInHalfspace(b, c, p);
			int w1 = pointInHalfspace(c, a, p);
			int w2 = pointInHalfspace(a, b, p);

			// determine barycentric coords
			vec3 lambda;

			lambda.x = (float)w0 / (w0+w1+w2);
			lambda.y = (float)w1 / (w0+w1+w2);
			lambda.z = (float)w2 / (w0+w1+w2);

			// calculate depth here
			float z = lambda.x*posA_win.z + lambda.y*posB_win.z + lambda.z*posC_win.z;

			if (w0 >= 0 && w1 >= 0 && w2 >= 0)
			{

				// depth test
				if (depthbuffer && (depthbuffer->conditionalPlot(x, y, z)))
				{		
					ShadingGeometry sgeo = triangle.rasterise( lambda );
					sgeo.windowCoord = p;
					sgeo.depth = z;

					Colour c = fragmentShader->shadeSingle(sgeo);
					framebuffer->plot(p, c);
				}

			}

		}
	}

}
