#include "RenderTarget.h"
#include "Exceptions.h"

RenderTarget::RenderTarget(unsigned int w, unsigned int h) : depthTest(true), width(w), height(h)
{
	pixels = new Colour[width*height];
	depth = new float[width*height];
}

RenderTarget::RenderTarget(const RenderTarget& rt) : depthTest(rt.depthTest), width(rt.width), height(rt.height)
{
	pixels = new Colour[width*height];
	depth = new float[width*height];
}

RenderTarget::~RenderTarget()
{
	delete[] pixels;
	delete[] depth;
}

void RenderTarget::clear(const Colour& clearColor, float clearDepth)
{
	for (unsigned int i = 0; i < width*height; ++i)
	{
		pixels[i].set(clearColor);
		depth[i] = clearDepth;
	}
}

bool RenderTarget::putPixel(unsigned int x, unsigned int y, const Colour& c)
{
	if (x > width-1 || y > height-1)
		throw OutofBoundsException(x, y);

	pixels[y*width+x].set(c);

	return true;
}


bool RenderTarget::putPixel(unsigned int x, unsigned int y, float z, const Colour& c)
{
	if (x > width-1 || y > height-1)
		throw OutofBoundsException(x, y);

	if (depth[y*width+x] < z)
	{
		pixels[y*width+x].set(c);
		depth[y*width+x] = z;
		return true;
	}


	return false;
}

