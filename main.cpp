#include <iostream>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#include <cstdlib>

#include "Framebuffer.h"
#include "Depthbuffer.h"
#include "Colour.h"
#include "Vertex.h"
#include "Shader.h"
#include "Rasteriser.h"
#include "Viewport.h"
#include "Geometry.h"
#include "Camera.h"
#include "CubeGeometry.h"
#include "RandomTriangleGeometry.h"
#include "GridGeometry.h"

#include "glmHelper.h"

unsigned int 	texture;

unsigned int 	width = 640, height = 480;

Geometry*		geometry = nullptr;
std::unique_ptr<CubeGeometry> cube = nullptr;
std::unique_ptr<RandomTriangleGeometry> triangles = nullptr;
std::unique_ptr<GridGeometry> grid;

Framebuffer*	framebuffer;
Depthbuffer*	depthbuffer;

Rasteriser*		rasteriser;
VertexShader*	vertexTransform;
FragmentShader*	fragmentShader;
FragmentShader*	normalShader;

Viewport*		viewport;

Camera*			camera;

bool rotate = false;
float rotationAngle = 0.f;

glm::ivec2 mousePosition;

static const int GLUT_MOUSEWHEEL_DOWN = 3;
static const int GLUT_MOUSEWHEEL_UP = 4;

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

	// update time
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

	// update animation
	if (rotate)
		rotationAngle += 1*dt;

	// clear the buffers	
	Colour clearColour(0, 0, 0.2f, 1.f);
	framebuffer->clear( clearColour );
	depthbuffer->clear();

	// reset the render matrices
	DefaultVertexTransform* dvt = reinterpret_cast<DefaultVertexTransform*>(rasteriser->vertexShader);
	dvt->modelMatrix = glm::mat4(1.f);
	dvt->viewMatrix = camera->getViewMatrix();
	dvt->projectionMatrix = camera->getProjectionMatrix();

	glm::mat4 rotate = glm::rotate(rotationAngle, glm::vec3(0.f, 1.f, 0.f));
	dvt->modelMatrix *= rotate;

	try
	{
		if (triangles)
		{
			rasteriser->drawTriangles(triangles->getVertices(), triangles->getIndices());
		}

		if (grid)
		{
			rasteriser->drawLines(grid->getVertices(), grid->getIndices());
		}
		
		if (cube)
		{
			rasteriser->drawLines(cube->getVertices(), cube->getIndices());
		}

		if (geometry)
		{
			DefaultVertexTransform* dvt = reinterpret_cast<DefaultVertexTransform*>(rasteriser->vertexShader);
			dvt->modelMatrix *= geometry->transform;

			rasteriser->fragmentShader = normalShader;
			rasteriser->drawTriangles( geometry->vertices, geometry->indices );
			rasteriser->fragmentShader = fragmentShader;
		}
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

	if (key == 't')
	{
		if (!triangles) 
			triangles = std::make_unique<RandomTriangleGeometry>(glm::vec3(-10, -10, -10), glm::vec3(10, 10, 10));			
		triangles->addTriangle();
	}

	if (key == 'b') 
	{
		rasteriser->toggleBoundingBoxes();
	}

	if (key == 'c')
	{
		// create a cube
		cube = std::make_unique<CubeGeometry>(glm::vec3(2.f, 2.f, 2.f));		
	}

	if (key == 'g')
	{
		delete geometry;
		geometry = new Geometry;

		geometry->loadPLY("models/bunny/reconstruction/bun_zipper_res3.ply");
		geometry->transform = glm::scale(glm::vec3(25.f, 25.f, 25.f));
		geometry->center();

	}

	if (key == 'G')
	{
		delete geometry;
		geometry = 0;
	}

	// single step advance
	if (key == 's')
	{
		rotationAngle += 1;

		glm::mat4 rotate = glm::rotate(rotationAngle, glm::vec3(0.f, 1.f, 0.f));
		DefaultVertexTransform* dvt = reinterpret_cast<DefaultVertexTransform*>(rasteriser->vertexShader);
		dvt->viewMatrix *= rotate;

	}

	if (key == 'S')
	{
		rotationAngle -= 1;

		glm::mat4 rotate = glm::rotate(rotationAngle, glm::vec3(0.f, 1.f, 0.f));		
		DefaultVertexTransform* dvt = reinterpret_cast<DefaultVertexTransform*>(rasteriser->vertexShader);
		dvt->viewMatrix *= rotate;
	}

	if (key == 'a' || key == 'z') {
		camera->handleKeyPress(key);
	}

}

static void motion(int x, int y)
{
	glm::ivec2 current(x, y);
	glm::ivec2 delta = current - mousePosition;
	mousePosition = current;

	camera->handleMouseMove(delta);
}

static void mouse(int button, int state, int x, int y)
{
	mousePosition = glm::ivec2(x, y);

	if (button == GLUT_MOUSEWHEEL_DOWN && state == GLUT_DOWN)
	{
		camera->handleKeyPress('a');

	}

	if (button == GLUT_MOUSEWHEEL_UP && state == GLUT_DOWN)
	{
		camera->handleKeyPress('z');

	}
}

static void cleanup()
{
	delete framebuffer;
	delete depthbuffer;

	delete vertexTransform;
	delete fragmentShader;
	delete normalShader;

	delete rasteriser;

	delete geometry;

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
	glutMotionFunc(motion);
	glutMouseFunc(mouse);

	atexit(cleanup);

	srand( time(0) );

	rasteriser = new Rasteriser;
	viewport = new Viewport(0, 0, width, height);
	rasteriser->viewport = viewport;

	framebuffer = new Framebuffer(width, height);
	rasteriser->framebuffer = framebuffer;

	depthbuffer = new Depthbuffer(width, height);
	rasteriser->depthbuffer = depthbuffer;

	DefaultVertexTransform* dvt = new DefaultVertexTransform;
	rasteriser->vertexShader = dvt;

	grid = std::make_unique<GridGeometry>();

	fragmentShader = new InputColourShader;
	rasteriser->fragmentShader = fragmentShader;

	normalShader = new NormalColourShader;

	camera = new OrbitCamera(glm::vec3(0,0,0), glm::vec3(0,1,0), 10.0f);

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
