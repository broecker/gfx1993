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
#include "Vertex.h"
#include "Shader.h"
#include "Rasteriser.h"
#include "Viewport.h"
#include "Geometry.h"
#include "Camera.h"

#include "glmHelper.h"

unsigned int 	texture;

unsigned int 	width = 640, height = 480;

VertexList 	vertices; 
IndexList	indices;

VertexList	triVertices;
IndexList	triIndices;

Geometry*		geometry = nullptr;

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

static Vertex createRandomVertex()
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

	return Vertex(glm::vec4(x,y,z,1.f), glm::normalize(glm::vec3(nx, ny, nz)), glm::vec4(r,g,b,1.f), texcoord);		
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
		rotationAngle += 15*dt;

	// clear the buffers	
	Colour clearColour(0, 0, 0.2f, 1.f);
	framebuffer->clear( clearColour );
	depthbuffer->clear();

	// reset the render matrices
	DefaultVertexTransform* dvt = reinterpret_cast<DefaultVertexTransform*>(rasteriser->vertexShader);
	dvt->modelMatrix = glm::mat4(1.f);
	dvt->viewMatrix = camera->getCameraMatrix();
	std::cout << "ViewMatrix: " << camera->getCameraMatrix() << std::endl;

	glm::mat4 rotate = glm::rotate(rotationAngle, glm::vec3(0.f, 1.f, 0.f));
	dvt->modelMatrix *= rotate;

	try
	{
		rasteriser->drawPoints( vertices );
		
		if (!indices.empty())
		{
			rasteriser->drawLines(vertices, indices);
		}		

		if (!triIndices.empty())
		{
			rasteriser->drawTriangles(triVertices, triIndices);
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

	if (key == 'p')
	{
		size_t pcount = 32;
		for (int i = 0; i < pcount; ++i)
		{
			vertices.push_back( createRandomVertex() );
		}

		std::clog << "Created " << vertices.size() << " vertices.\n";

	}

	if (key == 't')
	{
		Vertex a = createRandomVertex();
		Vertex b = createRandomVertex();
		Vertex c = createRandomVertex();

		triVertices.push_back(a);
		triIndices.push_back( triVertices.size()-1 );

		triVertices.push_back(b);
		triIndices.push_back( triVertices.size()-1 );

		triVertices.push_back(c);
		triIndices.push_back( triVertices.size()-1 );

		// generate lines
		/*
		vertices.push_back( a ); unsigned int ai = vertices.size()-1;
		vertices.push_back( b ); unsigned int bi = vertices.size()-1;
		vertices.push_back( c ); unsigned int ci = vertices.size()-1;

		indices.push_back( ai );
		indices.push_back( bi );
		indices.push_back( bi );
		indices.push_back( ci );
		indices.push_back( ci );
		indices.push_back( ai );
		*/
	}
	
	if (key == 'c')
	{
		vertices.clear();
		indices.clear();

		triVertices.clear();
		triIndices.clear();

		// create a cube
		vertices.push_back( Vertex( glm::vec4(-1,  1, 1, 1) ) );
		vertices.push_back( Vertex( glm::vec4(-1, -1, 1, 1) ) );
		vertices.push_back( Vertex( glm::vec4( 1, -1, 1, 1) ) );
		vertices.push_back( Vertex( glm::vec4( 1,  1, 1, 1) ) );
		vertices.push_back( Vertex( glm::vec4(-1,  1, -1, 1) ) );
		vertices.push_back( Vertex( glm::vec4(-1, -1, -1, 1) ) );
		vertices.push_back( Vertex( glm::vec4( 1, -1, -1, 1) ) );
		vertices.push_back( Vertex( glm::vec4( 1,  1, -1, 1) ) );

		for (int i = 0; i < 8; ++i)
		{
			float r = (float)rand() / RAND_MAX;
			float g = (float)rand() / RAND_MAX;
			float b = 1.f - (r+g);
			vertices[i].colour.r = r; 
			vertices[i].colour.g = g; 
			vertices[i].colour.b = b; 
		}

		indices.push_back(0); indices.push_back(1);
		indices.push_back(1); indices.push_back(2);
		indices.push_back(2); indices.push_back(3);
		indices.push_back(3); indices.push_back(0);
		indices.push_back(4); indices.push_back(5);
		indices.push_back(5); indices.push_back(6);
		indices.push_back(6); indices.push_back(7);
		indices.push_back(7); indices.push_back(4);
		indices.push_back(0); indices.push_back(4);
		indices.push_back(1); indices.push_back(5);
		indices.push_back(2); indices.push_back(6);
		indices.push_back(3); indices.push_back(7);
		
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

	dvt->modelMatrix = glm::mat4(1.f);
	dvt->viewMatrix[3] = glm::vec4(0, 0, -3.f, 1);
	dvt->projectionMatrix = glm::perspective(65.f, 1.3f, 1.f, 100.f);
	
	fragmentShader = new InputColourShader;
	rasteriser->fragmentShader = fragmentShader;

	normalShader = new NormalColourShader;

	camera = new OrbitCamera(glm::vec3(0,0,0), glm::vec3(0,1,0), 10.0f);

	vertices.push_back( Vertex(glm::vec4(0,0,0,1), glm::vec3(0,0,1), glm::vec4(1,1,1,1), glm::vec2(0,0)) );

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
