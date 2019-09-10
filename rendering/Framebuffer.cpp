#include "Framebuffer.h"

namespace render 
{

Framebuffer::Framebuffer(unsigned int w, unsigned int h) : width(w), height(h) 
{
	data = new Colour[width*height];
}

Framebuffer::~Framebuffer()
{
	delete[] data;
}

void Framebuffer::clear(const Colour& c) 
{
	for (unsigned int i = 0; i < width*height; ++i)
	{
		data[i].set( c );
	}
}

void Framebuffer::plot(unsigned int x, unsigned int y, const Colour& c) 
{
	if (x < width && y < height) 
	{
		data[x + y*width].set( c );
	}
}

}