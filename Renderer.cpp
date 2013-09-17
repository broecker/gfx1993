#include "Renderer.h"
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

unsigned int Renderer::drawPoints(const VertexList& vertices) const
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

void Renderer::transformVertices(const VertexList& vertices, VertexOutList& out) const
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

unsigned int Renderer::drawLines(const VertexList& vertices, const IndexList& indices) const
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

unsigned int Renderer::drawTriangles(const VertexList& vertices, const IndexList& indices) const
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
	CallVertexShader vertexTransform(vertexShader);

	std::transform(vertices.begin(), vertices.end(), transformedVertices.begin(), vertexTransform);

	// build triangles
	TrianglePrimitiveList triangles;
	for (int i = 0; i < indices.size(); i += 3)
	{
		const VertexOut& a = transformedVertices[ indices[i+0] ];
		const VertexOut& b = transformedVertices[ indices[i+1] ];
		const VertexOut& c = transformedVertices[ indices[i+2] ];


		// backface culling
		vec3 n = cross(vec3(b.clipPosition-a.clipPosition), vec3(c.clipPosition-a.clipPosition));
		if (dot(n, vec3(0, 0, 1)) > 0)
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
void Renderer::drawLine(const LinePrimitive& line) const
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

void Renderer::drawTriangle(const TrianglePrimitive& t) const
{

}
