#include <iostream>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/transform.hpp>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cstdlib>

#include "Framebuffer.h"
#include "Depthbuffer.h"
#include "Colour.h"
#include "Line.h"
#include "Vertex.h"
#include "Shader.h"
#include "Renderer.h"
#include "Viewport.h"

#include "glmHelper.h"

unsigned int 	texture;

unsigned int 	width = 320, height = 240;

VertexList 	vertices; 
IndexList	indices;

Framebuffer*	framebuffer;
Depthbuffer*	depthbuffer;

Renderer*		renderer;
VertexShader*	vertexTransform;
FragmentShader*	fragmentShader;

Viewport*		viewport;


bool rotate = false;
float rotationAngle = 0.f;

float animateBG = 0.f;
Colour bgColours[2];

static void setRandomBgColour(unsigned int index) 
{
	if (index > 1)
		return;

	float r = (float)rand()/RAND_MAX;
	float g = (float)rand()/RAND_MAX;
	float b = 1.f - r - g;

	bgColours[index].set(r,g,b,1.f);
}

static void display()
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, framebuffer->getPixels());

	glEnable(GL_TEXTURE_2D);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

	const int vertices[] = {0,0, 1,0, 1,1, 0,1};
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
		DefaultVertexTransform* dvt = reinterpret_cast<DefaultVertexTransform*>(renderer->vertexShader);
		dvt->modelMatrix = rotate;
	}

	animateBG += dt;
	if (animateBG > 3.f)
	{
		bgColours[0].set(bgColours[1]);
		setRandomBgColour( 1 );
		animateBG = 0.f;
	}

	
	// clear the buffers	
	Colour clearColour = Colour::lerp(bgColours[0], bgColours[1], animateBG/3.f);
	framebuffer->clear( clearColour );
	depthbuffer->clear();

	try
	{
		renderer->drawPoints( vertices );
		
		if (!indices.empty())
			renderer->drawLines(vertices, indices);
				
	}
	catch (const char* txt)
	{
		std::cerr << "Render error :\"" << txt << "\"\n";
	}


	glutPostRedisplay();
}

static void keyboard(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);

	if (key == 'r')
		rotate = !rotate;

	if (key == ' ')
	{
		unsigned int x = rand()%width;
		unsigned int y = rand()%height;
		unsigned int c = (float)rand()/RAND_MAX;

		std::clog << "Putting pixel at " << x << "," << y << std::endl;

	}

	if (key == 'l')
	{
		indices.clear();
		for (unsigned int i = 0; i < vertices.size()/2; ++i)
		{
			indices.push_back( i );
			
			// find a target vertex that is not our current one
			unsigned int target = i;
			while (target == i)
				target = rand() % vertices.size();
			
			indices.push_back( target );
			
			
		}
		
	}
	
	if (key == 'p')
	{
		size_t pcount = 32;
		for (int i = 0; i < pcount; ++i)
		{
			float x = (float)rand() / RAND_MAX * 2.f - 1.f;
			float y = (float)rand() / RAND_MAX * 2.f - 1.f;
			float z = (float)rand() / RAND_MAX * 2.f - 1.f;

			float nx = (float)rand() / RAND_MAX * 2.f - 1.f;
			float ny = (float)rand() / RAND_MAX * 2.f - 1.f;
			float nz = (float)rand() / RAND_MAX * 2.f - 1.f;

			glm::vec2 texcoord(0,0);

			float r = (float)rand() / RAND_MAX;
			float g = (float)rand() / RAND_MAX;
			float b = 1.f - r - g;

			vertices.push_back( Vertex(glm::vec4(x,y,z,1.f), glm::normalize(glm::vec3(nx, ny, nz)), glm::vec4(r,g,b,1.f), texcoord));
		
			std::clog << "Created point at (" << x << "," << y << "," << z << ")" << std::endl; 
		}

		std::clog << "Created " << vertices.size() << " vertices.\n";

	}

	if (key == 'c')
	{
		vertices.clear();
	}

}

static void cleanup()
{
	delete framebuffer;
	delete depthbuffer;

	delete vertexTransform;
	delete renderer;

	delete viewport;
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

	srand( time(0) );

	renderer = new Renderer;
	viewport = new Viewport(0, 0, width, height);
	renderer->viewport = viewport;

	framebuffer = new Framebuffer(width, height);
	renderer->framebuffer = framebuffer;

	depthbuffer = new Depthbuffer(width, height);
	renderer->depthbuffer = depthbuffer;

	DefaultVertexTransform* dvt = new DefaultVertexTransform;
	renderer->vertexShader = dvt;

	dvt->modelMatrix = glm::mat4(1.f);
	dvt->viewMatrix[3] = glm::vec4(0, 0, -10.f, 1);
	dvt->projectionMatrix = glm::perspective(45.f, 1.3f, 1.f, 100.f);

	/*
	float l = 0.f;
	float r = 1.f;
	float b = 0.f;
	float t = 1.f;
	float n = 0.f;
	float f = 1.f;

	float tx = -(r+l)/(r-l);
	float ty = -(t+b)/(t-b);
	float tz = -(f+n)/(f-n);


	dvt->projectionMatrix = glm::mat4(	glm::vec4(2.f/(r-l), 0, 0, 0), 
										glm::vec4(0, 2.f/(t-b), 0, 0),
										glm::vec4(0, 0, -2.f/(f-n), 0.f),
										glm::vec4(tx, ty, tz, 1.f));
	*/
	
	fragmentShader = new InputColourShader;
	renderer->fragmentShader = fragmentShader;


	vertices.push_back( Vertex(glm::vec4(0,0,0,1), glm::vec3(0,0,1), glm::vec4(1,1,1,1), glm::vec2(0,0)) );
	
	setRandomBgColour( 0 );
	setRandomBgColour( 1 );



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
