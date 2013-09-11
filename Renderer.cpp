#include "Renderer.h"
#include "Framebuffer.h"
#include "Depthbuffer.h"
#include "Viewport.h"

#include "Shader.h"
#include "glmHelper.h"

#include <algorithm>
#include <iostream>


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

unsigned int Renderer::drawLines(const VertexList& vertices, const IndexList& indices) const
{
	unsigned int linesDrawn = 0;


	// transform vertices

	// line assembly

	// clip lines

	// rasterize


	return linesDrawn;
}