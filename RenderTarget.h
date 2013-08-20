#ifndef RENDERTARGET_INCLUDED
#define RENDERTARGET_INCLUDED

#include "Colour.h"

struct Colour;

class RenderTarget
{
public:
	RenderTarget(unsigned int width, unsigned int height);
	RenderTarget(const RenderTarget& cp);
	virtual ~RenderTarget();

	void clear(const Colour& clearColour, float clearDepth);
	
	bool putPixel(unsigned int x, unsigned int y, const Colour& c);
	bool putPixel(unsigned int x, unsigned int y, float z, const Colour& c);

	inline const Colour* getPixels() const { return pixels; }
	const Colour& getPixel(unsigned int x, unsigned int y) const;
	float getDepth(unsigned int x, unsigned int y);

	inline unsigned int getWidth() const { return width; }
	inline unsigned int getHeight() const { return height; }

	bool depthTest;


protected:
	unsigned int 	width, height;

	Colour*			pixels;
	float*			depth;


};

#endif