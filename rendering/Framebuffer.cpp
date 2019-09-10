#include "Framebuffer.h"

namespace render 
{

Framebuffer::Framebuffer(unsigned int w, unsigned int h) : width(w), height(h) 
{
	data = new Color[width * height];
}

Framebuffer::~Framebuffer()
{
	delete[] data;
}

void Framebuffer::clear(const Color& c) 
{
	for (unsigned int i = 0; i < width*height; ++i)
	{
		data[i].set( c );
	}
}

void Framebuffer::plot(int x, int y, const Color& c)
{
    assert(x >= 0 && x < width);
    assert(y >= 0 && y < height);

	if (x < width && y < height) 
	{
		data[x + y*width].set( c );
	}
}

}