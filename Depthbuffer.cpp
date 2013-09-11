#include "Depthbuffer.h"

#include <cfloat>

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
	unsigned int x = (unsigned int)pos.x;
	unsigned int y = (unsigned int)pos.y;
	float z = pos.z;
	return conditionalPlot(x, y, z);
}

bool Depthbuffer::conditionalPlot(unsigned int x, unsigned int y, float z)

{	unsigned int i = x + width*y;
	if (data[i] > z)
	{
		data[x] = z;
		return true;
	}
	else
		return false;
}