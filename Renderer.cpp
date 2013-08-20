#include "Renderer.h"
#include "Line.h"
#include "RenderTarget.h"
#include "Viewport.h"

Renderer::Renderer(unsigned int w, unsigned int h)
{
	renderTarget = new RenderTarget(w, h);
	clearColour.setBlack();

	viewport = new Viewport(0, 0, w, h);
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
