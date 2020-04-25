#include "GlutDemoApp.h"

#include <cstdlib>
#include <iostream>

#include <GL/glut.h>
#include <GL/gl.h>

static const int GLUT_MOUSEWHEEL_DOWN = 3;
static const int GLUT_MOUSEWHEEL_UP = 4;

static const int WIDTH_VGA = 640;
static const int HEIGHT_VGA=  480;

GlutDemoApp *GlutDemoApp::appInstance = nullptr;

GlutDemoApp::GlutDemoApp(const std::string &name) : name(name), width(WIDTH_VGA), height(HEIGHT_VGA),
                                                                           texture(0), logFrameTime(true)
{
    rasterizer = std::make_unique<render::Rasterizer>();

    renderConfig.viewport = std::make_shared<render::Viewport>(0, 0, width, height);
    renderConfig.framebuffer = std::make_shared<render::Framebuffer>(width, height);
    renderConfig.depthbuffer = std::make_shared<render::Depthbuffer>(width, height);

    camera = std::make_unique<OrbitCamera>(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 30.0f);

    srand(time(0));
}

GlutDemoApp::~GlutDemoApp() {}


void GlutDemoApp::run(int argc, char **argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow(name.c_str());

    // Ugly hack :-/
    appInstance = this;

    glutDisplayFunc(GlutDemoApp::glutDisplay);
    glutIdleFunc(GlutDemoApp::glutIdle);
    glutKeyboardFunc(GlutDemoApp::glutKeyboard);
    glutMotionFunc(GlutDemoApp::glutMotion);
    glutMouseFunc(GlutDemoApp::glutMouse);

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

    this->init();

    glutMainLoop();
}

void GlutDemoApp::glutIdle()
{
    // update time
    static unsigned int oldTime = 0;

    int elapsedTime = glutGet(GLUT_ELAPSED_TIME);
    float dt = (float) (elapsedTime - oldTime) / 1000.f;
    oldTime = elapsedTime;

    static float timer = 0.f;
    static unsigned int frames = 0;
    timer += dt;
    ++frames;
    if (timer >= 2.f) {
        if (appInstance->logFrameTime)
            std::clog << "dt: " << dt << " " << frames / 2 << " fps" << std::endl;

        timer = 0.f;
        frames = 0;
    }

    appInstance->updateFrame(dt);
    appInstance->renderFrame();

    glutPostRedisplay();
}

void GlutDemoApp::glutDisplay()
{
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //if (appInstance->renderTarget.framebuffer)
    {
        glBindTexture(GL_TEXTURE_2D, appInstance->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, appInstance->width, appInstance->height, 0, GL_RGBA, GL_FLOAT,
                     appInstance->renderConfig.framebuffer->getPixels());

        glEnable(GL_TEXTURE_2D);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        const int vertices[] = {0, 0, 1, 0, 1, 1, 0, 1};
        glVertexPointer(2, GL_INT, 0, vertices);
        glTexCoordPointer(2, GL_INT, 0, vertices);

        glDrawArrays(GL_QUADS, 0, 4);

    }
    glutSwapBuffers();
}

void GlutDemoApp::glutKeyboard(unsigned char key, int x, int y)
{
    // Escape
    if (key == 27)
        exit(0);

    appInstance->handleKeyboard(key, x, y);
}

void GlutDemoApp::glutMotion(int x, int y)
{
    appInstance->handleMotion(x, y);
}

void GlutDemoApp::glutMouse(int button, int state, int x, int y)
{
    appInstance->handleMouse(button, state, x, y);
}

void GlutDemoApp::handleKeyboard(unsigned char key, int x, int y) {}

void GlutDemoApp::handleMouse(int button, int state, int x, int y)
{
    mousePosition = glm::ivec2(x, y);

    if (button == GLUT_MOUSEWHEEL_DOWN && state == GLUT_DOWN) {
        camera->handleKeyPress('a');
    }

    if (button == GLUT_MOUSEWHEEL_UP && state == GLUT_DOWN) {
        camera->handleKeyPress('z');
    }
}

void GlutDemoApp::handleMotion(int x, int y)
{
    glm::ivec2 current(x, y);
    glm::ivec2 delta = current - mousePosition;
    mousePosition = current;

    camera->handleMouseMove(delta);
}