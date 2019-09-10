#ifndef FRAMEBUFFER_INCLUDED
#define FRAMEBUFFER_INCLUDED

#include "../common/Color.h"

#include <glm/glm.hpp>

namespace render 
{

class Framebuffer
{
public:
	Framebuffer(unsigned int w, unsigned int h);
	Framebuffer(const Framebuffer& cp);
 
	virtual ~Framebuffer();

	virtual void clear(const Color& c);

	inline void plot(const glm::ivec2& p, const Color& c) { this->plot(p.x, p.y, c); }
	void plot(int x, int y, const Color& c);


	inline unsigned int getWidth() const { return width; }
	inline unsigned int getHeight() const { return height; }
	inline const Color* getPixels() const { return data; }


protected:
	unsigned int 		width, height;
	Color*				data;

};

}

#endif