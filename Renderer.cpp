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

Renderer::Renderer() : vertexShader(0), fragmentShader(0), framebuffer(0), depthbuffer(0), viewport(0), frustum(0)
{
}

unsigned int Renderer::drawPoints(const VertexList& vertices) const
{
	using namespace glm;

	if (!vertexShader)
		throw "Vertex shader missing!";

	if (!fragmentShader)
		throw "Fragment shader missing!";

	if (!framebuffer)
		throw "Framebuffer missing!";

	if (!depthbuffer)
		throw "Depth buffer missing!";

	if (!viewport)
		throw "Viewport is missing!";

	unsigned int pointsDrawn = 0;

	// single vertex pipeline

	// transform vertices
	for (VertexList::const_iterator v = vertices.begin(); v != vertices.end(); ++v)
	{
		VertexOut transformedVertex = vertexShader->transformSingle(*v);

		const vec4& pos_clip = transformedVertex.clipPosition;

		// clipping
		if (pos_clip.x >= -pos_clip.w && pos_clip.x <= pos_clip.w &&
			pos_clip.y >= -pos_clip.w && pos_clip.y <= pos_clip.w && 
			pos_clip.z >= -pos_clip.w && pos_clip.z <= pos_clip.w)
		{

			// perspective divide to normalised device coordinates
			vec3 pos_ndc(pos_clip.x/pos_clip.w, pos_clip.y/pos_clip.w, pos_clip.z/pos_clip.w);

			// screen coords
			vec3 pos_win = viewport->calculateWindowCoordinates( pos_ndc );

			unsigned int x = pos_win.x;
			unsigned int y = pos_win.y;

			ShadingGeometry sgeo;
			sgeo.worldPosition = transformedVertex.worldPosition;
			sgeo.normal = transformedVertex.worldNormal;
			sgeo.colour = transformedVertex.colour;
			sgeo.windowCoord = ivec2(x, y);

			if (depthbuffer->conditionalPlot(x,y, pos_win.z))
			{
				++pointsDrawn;
				
				Colour result = fragmentShader->shadeSingle( sgeo );
				framebuffer->plot(x, y, result );
			}
		}
		else
		{
			// clipped!
			continue;
		}

	}

	return pointsDrawn;
}

unsigned int Renderer::drawLines(const VertexList& vertices, const IndexList& indices) const
{
	unsigned int linesDrawn = 0;

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

	// build lines
	LinePrimitiveList lines;
	for (int i = 0; i < indices.size(); i += 2)
	{
		const VertexOut& a = transformedVertices[ indices[i+0] ];
		const VertexOut& b = transformedVertices[ indices[i+1] ];
		lines.push_back( LinePrimitive(a, b) );
	}
	
	LinePrimitiveList::iterator l = lines.begin();
	while( l != lines.end() )
	{
		// clip and cull lines here


		// depth test


		// rasterise

		// create shading geometry

		// fragment shader



		++l;
	}

	return linesDrawn;
}

void Renderer::bresenhamLine(glm::ivec2 a, glm::ivec2 b, glm::vec4 colour)
{
	if (!framebuffer)
		throw "Framebuffer missing!";
	
	if (!depthbuffer)
		throw "Depth buffer missing!";
	
	/*
	if (!fragmentShader)
		throw "Fragment shader missing!";
	*/
	
	using namespace glm;
	
	glm::ivec2 d = b-a;
	glm::ivec2 s(-1, -1);
	
	if (a.x < b.x) s.x = 1;
	if (a.y < b.y) s.y = 1;
	int err = d.x-d.y;
	
	do {
		
		// TODO: insert depth test here
		// TODO: interpolate colour, etc
		// TODO: run fragment shader
		framebuffer->plot(a.x, a.y, Colour(colour));
		
		int e2 = 2*err;
		if (e2 > err - d.y)
		{
			err -= d.y;
			a.x += s.x;
		}
		
		if (a == b)
		{
			framebuffer->plot(a.x, a.y, Colour(colour));
			return;
		}
		
		if (e2 < d.x)
		{
			err = err + d.x;
			a.y += s.y;
		}
		
		
	} while (a != b);
	
}
