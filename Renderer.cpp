#include "Renderer.h"
#include "Framebuffer.h"
#include "Depthbuffer.h"
#include "Viewport.h"

#include "Shader.h"
#include "glmHelper.h"
#include "Line.h"
#include "Frustum.h"

#include <algorithm>
#include <iostream>
#include <list>

unsigned int Renderer::drawPoints(const VertexList& vertices) const
{
	using namespace glm;

	if (!vertexShader)
		throw "Vertex shader missing!";

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
		Vertex transformedVertex = vertexShader->transformSingle(*v);

		vec4 pos_clip = transformedVertex.position;

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

			if (depthbuffer->conditionalPlot(x,y, pos_win.z))
			{
				++pointsDrawn;
				framebuffer->plot(x, y, Colour(transformedVertex.colour));
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

unsigned int Renderer::drawLines(VertexList vertices, const IndexList& indices) const
{
	unsigned int linesDrawn = 0;

	using namespace glm;
	
	if (!vertexShader)
		throw "Vertex shader missing!";
	
	if (!viewport)
		throw "Viewport is missing!";
			
	if (!frustum)
		throw "Frustum is missing!";
	
	// transform vertices
	for (VertexList::iterator v = vertices.begin(); v != vertices.end(); ++v)
	{
		// transform vertex to clip space
		*v = vertexShader->transformSingle( *v );
	}
		
	// line assembly
	std::list<Line3D> lines;
	for (unsigned int i = 0; i < indices.size(); i += 2)
	{
		const Vertex& a = vertices[i+0];
		const Vertex& b = vertices[i+1];
				
		lines.push_back( Line3D(a,b) );
	}
	
	
	// for each of the six clipping planes:
	for (int p = 0; p < 6; ++p)
	{
		const Plane& clippingPlane = frustum->plane[p];
				
		std::list<Line3D>::iterator l = lines.begin();
		while (l != lines.end())
		{
			const Line3D& line = *l;
			
			
			// cull lines
			
			
			
			
			
			
			// clip lines
			
			
			
			// rasterize
			
			
		}
		

		
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
