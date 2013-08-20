#ifndef RENDERER_INCLUDED
#define RENDERER_INCLUDED

#include "Colour.h"

class RenderTarget;
class Viewport;

struct Line2D;

class Renderer
{
public:
	Renderer(unsigned int width, unsigned int height);
	virtual ~Renderer();


	void clearBuffers();

	void setClearColour(const Colour& c) { clearColour = c; }


	inline const RenderTarget& getRenderTarget() const { return *renderTarget; }


	void drawPixel(unsigned int x, unsigned int y, const Colour& c);
 	void drawLine(const Line2D& line);


protected:
 	RenderTarget*	renderTarget;
 	Viewport*		viewport;

 	Colour			clearColour;
};


#endif