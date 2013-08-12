
#include <iostream>
#include <cstdlib>

#include <GL/glew.h>
#include <GL/glut.h>

#include "Engine.h"


Engine*		gEngine = 0;


static void display()
{
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	gEngine->draw();

	glutSwapBuffers();
}

static void idle()
{
	static unsigned int oldTime = 0;
	unsigned int time = glutGet(GLUT_ELAPSED_TIME);

	float dt = std::min(1.f, float(time - oldTime) / 1000.f);
	oldTime = time;

	gEngine->update( dt );

}

static void keyboard(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);

}

static void cleanup()
{
	delete gEngine;
}

int main(int argc, char** argv)
{

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA|GLUT_DEPTH|GLUT_DOUBLE);
	glutInitWindowSize(800, 600);

	glutCreateWindow("Strikemission");
	glewInit();

	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0,0,0.2,0);


	gEngine = new Engine;

	atexit( cleanup );

	glutMainLoop();

	return 0;
}
