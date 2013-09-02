#include "Renderer.h"
#include "Line.h"
#include "RenderTarget.h"
#include "Viewport.h"

//#include <glm/ext.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

static std::ostream& operator << (std::ostream& os, const glm::vec4& v)
{
	return os << "(" << v.x << "," << v.y << "," << v.z << "," << v.w << ")";
}

static std::ostream& operator << (std::ostream& os, const glm::vec3& v)
{
	return os << "(" << v.x << "," << v.y << "," << v.z << ")";
}

static std::ostream& operator << (std::ostream& os, const glm::ivec2& v)
{
	return os << "(" << v.x << "," << v.y << ")";
}

Renderer::Renderer(unsigned int w, unsigned int h)
{
	renderTarget = new RenderTarget(w, h);
	clearColour.setBlack();

	viewport = new Viewport(0, 0, w, h);

	modelMatrix = glm::mat4(1.f);
	viewMatrix = glm::mat4(1.f);
	projectionMatrix = glm::ortho(0, 1, 0, 1, 0, 1);


	float l = 0.f;
	float r = 1.f;
	float b = 0.f;
	float t = 1.f;
	float n = 0.f;
	float f = 1.f;

	float tx = -(r+l)/(r-l);
	float ty = -(t+b)/(t-b);
	float tz = -(f+n)/(f-n);

	projectionMatrix = glm::mat4(	glm::vec4(2.f/(r-l), 0, 0, 0), 
									glm::vec4(0, 2.f/(t-b), 0, 0),
									glm::vec4(0, 0, -2.f/(f-n), 0.f),
									glm::vec4(tx, ty, tz, 1.f));

	//projectionMatrix = glm::perspective(45.f, 1.3f, 1.f, 100.f);

}

Renderer::~Renderer()
{
	delete renderTarget;
	delete viewport;
}

void Renderer::clearBuffers()
{
	float clearDepth = 1000.f;
	renderTarget->clear(clearColour, clearDepth);
}

void Renderer::drawPixel(unsigned int x, unsigned int y, const Colour& c)
{
	if (viewport->isInside(glm::ivec2(x,y)))
		renderTarget->putPixel(x, y, c);

}

void Renderer::drawPoints(const VertexList& vertices)
{
	using namespace glm;

	// transform all vertices
	mat4 mvp = projectionMatrix * viewMatrix * modelMatrix;

	for (VertexList::const_iterator it = vertices.begin(); it != vertices.end(); ++it)
	{
		const vec4& vertex = it->position; 

		// calculate clip coords:
		vec4 clipPos = mvp * vertex;
		//std::clog << vertex << " -> " << clipPos;  
		
		// perspective division
		vec3 ndc = vec3(clipPos.x, clipPos.y, clipPos.z) / clipPos.w;  
		//std::clog << " -> " << ndc;

		// clip
		if (ndc.x < -1.f || ndc.x > 1.f || ndc.y < -1.f || ndc.y > 1.f) // || ndc.z < -1.f || ndc.z > 1.f)
		{
			//std::clog << " ... clipped" << std::endl;
			continue;
		}


		// viewport transform
		vec3 winPos = viewport->calculateWindowCoordinates( ndc );

		ivec2 pixelPos = ivec2(winPos.x, winPos.y);
		if (viewport->isInside(pixelPos))
		{
			renderTarget->putPixel(pixelPos.x, pixelPos.y, winPos.z, Colour(it->colour));
		}

		//std::clog << " -> " << winPos << " -> " << pixelPos << "d: " << winPos.z << std::endl;
	}

}

void Renderer::drawLine(const Line2D& l)
{
	using namespace glm;

	// clip a and b to the max/min canvas
	if (!viewport->isInside(l))
		return;


	// clip line here


	// bresenham

	const Colour colour(1.f, 1.f, 0.f, 1.f);


	const ivec2 a(l.a);
	const ivec2 b(l.b);

	ivec2 d(abs(b.x-a.x), abs(b.y-a.y));

	// slope
	ivec2 s(-1, -1);
	if (a.x < b.x)
		s.x = 1;
	if (a.y < b.y)
		s.y = 1;

	int err = d.x-d.y;

	// current point on the line
	ivec2 p = a;
	do
	{
		renderTarget->putPixel(p.x, p.y, colour);
		
		// exit condition
		if (p.x == b.x && p.y == b.y)
			break;

		int err2 = err*2;
		if (err2 > -d.y)
		{
			err -= d.y;
			p.x += s.x;
		}

		if (p.x == b.x && p.y == b.y)
		{
			renderTarget->putPixel(p.x, p.y, colour);
			break;
		}

		if (err2 < d.x)
		{
			err += d.x;
			p.y += s.y;
		}


	} while (true);


}
