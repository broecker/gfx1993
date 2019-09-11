#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <GL/glut.h>

#include "rendering/Framebuffer.h"
#include "rendering/Depthbuffer.h"
#include "rendering/Pipeline.h"
#include "rendering/Shader.h"
#include "rendering/Rasterizer.h"
#include "rendering/Viewport.h"

unsigned int 	texture;
unsigned int 	width = 640, height = 480;

render::VertexList vertices;
render::IndexList indices;

render::Rasterizer::RenderConfiguration renderConfiguration;
std::unique_ptr<render::Rasterizer>		rasterizer;
std::shared_ptr<render::DefaultVertexTransform>	vertexShader;

using render::Vertex;

float rotationAngle = 0.f;

// Copies the rendered image back to OpenGL to display in a glut window.
static void display()
{
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, renderConfiguration.framebuffer->getPixels());

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

	int elapsedTime = glutGet(GLUT_ELAPSED_TIME);
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

    rotationAngle += dt * 3.f;

	// clear the buffers	
    renderConfiguration.framebuffer->clear( glm::vec4(0, 0, 0.2f, 1) );
    renderConfiguration.depthbuffer->clear();

	// reset the render matrices
    vertexShader->modelMatrix = glm::rotate(rotationAngle, glm::vec3(0,1,0));
    vertexShader->viewMatrix = glm::lookAt(glm::vec3(0, 1, -10), glm::vec3(0,0,0), glm::vec3(0,1,0));
    vertexShader->projectionMatrix = glm::perspective(glm::radians(60.f), (float)width/height, 0.2f, 100.f);

	try
	{
	    rasterizer->drawTriangles(vertices, indices, renderConfiguration);
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

	srand( time(0) );

    rasterizer = std::make_unique<render::Rasterizer>();
    renderConfiguration.viewport = std::make_shared<render::Viewport>(0, 0, width, height);
    renderConfiguration.framebuffer = std::make_shared<render::Framebuffer>(width, height);
    renderConfiguration.depthbuffer = std::make_shared<render::Depthbuffer>(width, height);

    // Keep a separate reference to the vertex shader so we can change the transform easily.
    vertexShader = std::make_shared<render::DefaultVertexTransform>();
    renderConfiguration.vertexShader = vertexShader;

    renderConfiguration.fragmentShader = std::make_shared<render::InputColourShader>();

    // Create the triangle geometry
    vertices.push_back(Vertex(glm::vec4(-5, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(0, 0, 1, 1), glm::vec2(0,0)));
    vertices.push_back(Vertex(glm::vec4( 0, 5, 0, 1), glm::vec3(1, 0, 0), glm::vec4(1, 0, 0, 1), glm::vec2(0,0)));
    vertices.push_back(Vertex(glm::vec4( 5, 0, 0, 1), glm::vec3(1, 0, 0), glm::vec4(0, 1, 0, 1), glm::vec2(0,0)));

    indices.push_back(0);
    indices.push_back(2);
    indices.push_back(1);
    indices.push_back(2);
    indices.push_back(0);
    indices.push_back(1);

    // OpenGL texture setup
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
