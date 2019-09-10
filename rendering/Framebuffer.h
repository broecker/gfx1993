#ifndef FRAMEBUFFER_INCLUDED
#define FRAMEBUFFER_INCLUDED

#include "../common/Colour.h"

#include <glm/glm.hpp>

namespace render 
{

class Framebuffer
{
public:
	Framebuffer(unsigned int w, unsigned int h);
	Framebuffer(const Framebuffer& cp);
 
	virtual ~Framebuffer();

	virtual void clear(const Colour& c);

	inline void plot(const glm::ivec2& p, const Colour& c) { this->plot(p.x, p.y, c); }
	void plot(unsigned int x, unsigned int y, const Colour& c);


	inline unsigned int getWidth() const { return width; }
	inline unsigned int getHeight() const { return height; }
	inline const Colour* getPixels() const { return data; }


protected:
	unsigned int 		width, height;
	Colour*				data;

};

}

#endif