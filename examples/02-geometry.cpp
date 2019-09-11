#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <GL/glut.h>

#include "rendering/Framebuffer.h"
#include "rendering/Depthbuffer.h"
#include "common/Color.h"
#include "common/Vertex.h"
#include "rendering/Shader.h"
#include "rendering/Rasterizer.h"
#include "rendering/Viewport.h"
#include "common/Camera.h"
#include "geometry/CubeGeometry.h"
#include "geometry/RandomTriangleGeometry.h"
#include "geometry/GridGeometry.h"
#include "geometry/PlyGeometry.h"

// OpenGL texture Id
unsigned int 	texture;
unsigned int 	width = 640, height = 480;

std::unique_ptr<geo::GridGeometry> grid;
std::vector<std::unique_ptr<geo::PlyGeometry> > bunnyList;

std::shared_ptr<render::Framebuffer>	framebuffer;
std::shared_ptr<render::Depthbuffer>	depthbuffer;

std::unique_ptr<render::Rasterizer>		rasterizer;
std::shared_ptr<render::DefaultVertexTransform>	vertexTransform;

std::unique_ptr<Camera>	camera;

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

	// clear the buffers	
	Color clearColour(0.7f, 00.7f, 1.f, 1.f);
	framebuffer->clear( clearColour );
	depthbuffer->clear();

	// reset the render matrices
    vertexTransform->modelMatrix = glm::mat4(1.f);
    vertexTransform->viewMatrix = camera->getViewMatrix();
    vertexTransform->projectionMatrix = camera->getProjectionMatrix();

	try
	{
	    // Draw the floor grid.
		rasterizer->drawLines(grid->getVertices(), grid->getIndices());

		// Draw all the bunnies.
		for (auto bunny = bunnyList.begin(); bunny != bunnyList.end(); ++bunny)
		{
            vertexTransform->modelMatrix = (*bunny)->transform;
			rasterizer->drawTriangles((*bunny)->getVertices(), (*bunny)->getIndices() );
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

	if (key == 'b') 
	{
		rasterizer->toggleBoundingBoxes();
	}

	if (key == 'g')
	{
		std::unique_ptr<geo::PlyGeometry> bunny = std::make_unique<geo::PlyGeometry>();

		bunny->loadPly("../models/bunny/reconstruction/bun_zipper_res3.ply");

		float randomAngle = (float)rand() / RAND_MAX;
		glm::vec3 randomAxis = glm::sphericalRand(1);

		const glm::vec4 minBounds(-15, -4, -15, 1);
		const glm::vec4 maxBounds( 15, 15, 15, 1);
		const glm::vec3 minScale(15, 15, 15);
		const glm::vec3 maxScale(30, 30, 30);

		bunny->transform = glm::rotate(randomAngle, randomAxis);
		bunny->transform[3] = glm::linearRand(minBounds, maxBounds);
		bunny->transform *= glm::scale(glm::linearRand(minScale, maxScale));

		bunnyList.emplace_back(std::move(bunny));
	}

	if (key == 'G')
    {
	    bunnyList.clear();
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

int main(int argc, char** argv)
{
	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA|GLUT_DOUBLE|GLUT_DEPTH);
	glutInitWindowSize(width,height);
	glutCreateWindow("srender");

	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMotionFunc(motion);
	glutMouseFunc(mouse);

	srand( time(0) );

    rasterizer = std::make_unique<render::Rasterizer>();
    rasterizer->viewport = std::make_shared<render::Viewport>(0, 0, width, height);

	framebuffer = std::make_shared<render::Framebuffer>(width, height);
    rasterizer->framebuffer = framebuffer;

	depthbuffer = std::make_shared<render::Depthbuffer>(width, height);
    rasterizer->depthbuffer = depthbuffer;

    vertexTransform = std::make_shared<render::DefaultVertexTransform>();
    rasterizer->vertexShader = vertexTransform;
    // We're not going to change the normal shader.
    rasterizer->fragmentShader = std::make_shared<render::NormalColourShader>();

	grid = std::make_unique<geo::GridGeometry>();



	camera = std::make_unique<OrbitCamera>(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 10.0f);

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
