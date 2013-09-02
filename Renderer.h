#ifndef RENDERER_INCLUDED
#define RENDERER_INCLUDED

#include <glm/glm.hpp>

#include "Colour.h"
#include "Vertex.h"

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


	void drawPoints(const VertexList& vertices);


	void drawPixel(unsigned int x, unsigned int y, const Colour& c);
 	void drawLine(const Line2D& line);



 	glm::mat4		modelMatrix;
 	glm::mat4		viewMatrix;
 	glm::mat4		projectionMatrix;

protected:
 	RenderTarget*	renderTarget;
 	Viewport*		viewport;

 	Colour			clearColour;
};


#endif