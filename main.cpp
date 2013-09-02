#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>

#include <GL/glut.h>

#include <cstdlib>

#include "Renderer.h"
#include "RenderTarget.h"
#include "Colour.h"
#include "Line.h"
#include "Vertex.h"

Renderer*		renderer;

unsigned int 	texture;

unsigned int 	width = 320, height = 240;

VertexList points; 

bool rotate = true;
float rotationAngle = 0.f;

float animateBG = 0.f;
Colour bgColours[2];

static void display()
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, renderer->getRenderTarget().getPixels());

	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	static int vertices[] = {0,0, 1,0, 1,1, 0,1};
	glVertexPointer(2, GL_INT, 0, vertices);
	glTexCoordPointer(2, GL_INT, 0, vertices);

	glDrawArrays(GL_QUADS, 0, 4);


	glutSwapBuffers();
}

static void idle()
{
	static unsigned int oldTime = 0;

	unsigned int elapsedTime = glutGet(GLUT_ELAPSED_TIME);
	float dt = (float)(elapsedTime-oldTime) / 1000.f;
	oldTime = elapsedTime;


	static float timer = 0.f;
	static unsigned int frames = 0;
	timer += dt;
	++frames;
	if (timer >= 2.f)
	{
		std::clog << "dt: " << dt << " " << frames/2 << " fps" << std::endl;

		timer = 0.f;
		frames = 0;
	}



	if (rotate)
	{
		rotationAngle += 5*dt;
		glm::mat4 rotate = glm::rotate(rotationAngle, 0.f, 1.f, 0.f);
		renderer->viewMatrix = rotate;
	}

	animateBG += dt;
	if (animateBG > 3.f)
	{
		bgColours[0].set(bgColours[1]);
		
		float r = (float)rand()/RAND_MAX;
		float g = (float)rand()/RAND_MAX;
		float b = 1.f - r - g;

		bgColours[1].set(r, g, b, 1.f);


		animateBG = 0.f;
	}

	renderer->setClearColour( Colour::lerp(bgColours[0], bgColours[1], animateBG/3.f));

	renderer->clearBuffers();
	renderer->drawPoints( points );

	glutPostRedisplay();
}

static void keyboard(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);

	if (key == 'c')
		renderer->clearBuffers();

	if (key == 'r')
		rotate = !rotate;

	if (key == ' ')
	{
		unsigned int x = rand()%width;
		unsigned int y = rand()%height;
		unsigned int c = (float)rand()/RAND_MAX;

		renderer->drawPixel(x, y, Colour((float)x/width, (float)y/height, c));

		std::clog << "Putting pixel at " << x << "," << y << std::endl;

	}

	if (key == 'l')
	{
		unsigned int x1 = rand()%width;
		unsigned int y1 = rand()%height;
		unsigned int x2 = rand()%width;
		unsigned int y2 = rand()%height;

		Line2D line(glm::vec2(x1, y1), glm::vec2(x2, y2));
		renderer->drawLine(line);
	
		std::clog << "Drawing line from (" << x1 << "," << y1 << ") to (" << x2 << "," << y2 << ")" << std::endl;
	}

	if (key == 'p')
	{
		size_t pcount = 25;
		points.clear();
		points.reserve( pcount );

		for (int i = 0; i < pcount; ++i)
		{
			float x = (float)rand() / RAND_MAX;
			float y = (float)rand() / RAND_MAX;
			float z = (float)rand() / RAND_MAX;

			float r = (float)rand() / RAND_MAX;
			float g = (float)rand() / RAND_MAX;
			float b = 1.f - r - g;

			points.push_back( Vertex(glm::vec4(x,y,z,1.f), glm::vec4(r,g,b,1.f)));
		
			std::clog << "Created point at (" << x << "," << y << "," << z << ")" << std::endl; 
		}

	}

}

static void cleanup()
{

}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(800,600);
	glutCreateWindow("srender");

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);

	atexit(cleanup);

	renderer = new Renderer(width, height);

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, 0);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, 1, 0, 1, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glutMainLoop();
	return 0;
}
