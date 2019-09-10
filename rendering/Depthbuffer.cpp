#include "Depthbuffer.h"

#include <cassert>
#include <cfloat>

namespace render 
{

Depthbuffer::Depthbuffer(unsigned int w, unsigned int h) : width(w), height(h)
{
	data = new float[width*height];
}

Depthbuffer::~Depthbuffer()
{
	delete[] data;
}

void Depthbuffer::clear()
{
	for (unsigned int i = 0; i < width*height; ++i) 
	{
		data[i] = FLT_MAX;
	}
}

bool Depthbuffer::conditionalPlot(const glm::vec3& pos)
{
	auto x = (unsigned int)pos.x;
	auto y = (unsigned int)pos.y;
	float z = pos.z;
	return conditionalPlot(x, y, z);
}

bool Depthbuffer::conditionalPlot(unsigned int x, unsigned int y, float z)
{	
	// TODO(mbroecker): This is off by one!
	assert(x <= width);
	assert(y <= height);
	unsigned int i = x + width*y;
	if (data[i] > z)
	{
		data[i] = z;
		return true;
	}
	else
		return false;
}

}