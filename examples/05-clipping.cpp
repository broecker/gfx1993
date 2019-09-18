#include <cstdlib>
#include <iostream>
#include <memory>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/polar_coordinates.hpp>
#include <glm/gtx/io.hpp>

#include <GL/glut.h>
#include <rendering/Pipeline.h>
#include <geometry/Geometry.h>

#include "rendering/Framebuffer.h"
#include "rendering/Depthbuffer.h"
#include "rendering/Shader.h"
#include "rendering/Rasterizer.h"
#include "rendering/Viewport.h"
#include "common/Camera.h"

// OpenGL texture Id
unsigned int texture;
unsigned int width = 640, height = 480;

class BoundedGeometry : public geo::Geometry {
public:
    BoundedGeometry(const glm::vec3& min, const glm::vec3& max) : maxBounds(max), minBounds(min) {}
    virtual ~BoundedGeometry() = default;

protected:
    glm::vec3           maxBounds, minBounds;

    static glm::vec4 getRandomColor() {
        float r = glm::linearRand(0.f, 1.f);
        float g = glm::linearRand(0.f, 1.f);
        float b = 1.f - r - g;
        float a = 1.f;
        return glm::vec4(r, g, b, a);
    }
};

class RandomLinesGeometry : public BoundedGeometry
{
public:
    RandomLinesGeometry(const glm::vec3& min, const glm::vec3& max) : BoundedGeometry(min, max) {}

    void add() {
        const glm::vec4 a(glm::linearRand(minBounds.x, maxBounds.x), glm::linearRand(minBounds.y, maxBounds.y), glm::linearRand(minBounds.z, maxBounds.z), 1.f);
        const glm::vec4 b(glm::linearRand(minBounds.x, maxBounds.x), glm::linearRand(minBounds.y, maxBounds.y), glm::linearRand(minBounds.z, maxBounds.z), 1.f);
        const glm::vec3 zero(0);


        vertices.push_back(render::Vertex(a, zero, getRandomColor(), glm::vec2(0)));
        vertices.push_back(render::Vertex(b, zero, getRandomColor(), glm::vec2(1)));
        indices.push_back(vertices.size()-2);
        indices.push_back(vertices.size()-1);
    }

    void clear() {
        vertices.clear();
        indices.clear();
    }
};

class RandomPointsGeometry : public BoundedGeometry
{
public:
    RandomPointsGeometry(const glm::vec3& min, const glm::vec3& max) : BoundedGeometry(min, max) {}

    void add() {
        const glm::vec4 p(glm::linearRand(minBounds.x, maxBounds.x), glm::linearRand(minBounds.y, maxBounds.y), glm::linearRand(minBounds.z, maxBounds.z), 1.f);
        const glm::vec3 zero(0);

        vertices.push_back(render::Vertex(p, zero, getRandomColor(), glm::vec2(0)));
        indices.push_back(vertices.size()-1);
    }

    void clear() {
        vertices.clear();
        indices.clear();
    }
};


std::unique_ptr<RandomLinesGeometry> lines;
std::unique_ptr<RandomPointsGeometry> points;

std::unique_ptr<render::Rasterizer> rasterizer;
render::Rasterizer::ShaderConfiguration shaders;
render::Rasterizer::RenderOutput renderTarget;

std::unique_ptr<Camera> camera;

glm::ivec2 mousePosition;

static const int GLUT_MOUSEWHEEL_DOWN = 3;
static const int GLUT_MOUSEWHEEL_UP = 4;

enum ProjectionMode {
    PERSPECTIVE,
    ORTHO
} projectionMode = PERSPECTIVE;

static void display()
{
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, renderTarget.framebuffer->getPixels());

    glEnable(GL_TEXTURE_2D);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    const int vertices[] = {0, 0, 1, 0, 1, 1, 0, 1};
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
    float dt = (float) (elapsedTime - oldTime) / 1000.f;
    oldTime = elapsedTime;

    static float timer = 0.f;
    static unsigned int frames = 0;
    timer += dt;
    ++frames;
    if (timer >= 2.f) {
        std::clog << "dt: " << dt << " " << frames / 2 << " fps" << std::endl;
        timer = 0.f;
        frames = 0;
    }

    // Clear the buffers
    renderTarget.framebuffer->clear(glm::vec4(0.7f, 0.7f, 0.9f, 1));
    renderTarget.depthbuffer->clear();

    // reset the render matrices
    render::DefaultVertexTransform *dvt = dynamic_cast<render::DefaultVertexTransform *>(shaders.vertexShader.get());

    dvt->modelMatrix = glm::mat4(1.f);
    dvt->viewMatrix = camera->getViewMatrix();

    switch (projectionMode) {
        case PERSPECTIVE:
            dvt->projectionMatrix = camera->getProjectionMatrix();
            break;
        case ORTHO:
            dvt->projectionMatrix = glm::ortho(-12.f, 12.f, -12.f, 12.f, -12.f, 12.f );
            break;
        default:
            assert(false);
    }

    try {
        rasterizer->drawLines(lines->getVertices(), lines->getIndices());
        rasterizer->drawPoints(points->getVertices(), points->getIndices());
    }
    catch (const char *txt) {
        std::cerr << "Render error :\"" << txt << "\"\n";
    }

    glutPostRedisplay();
}

static void keyboard(unsigned char key, int x, int y)
{
    if (key == 27) {
        exit(0);
    }

    if (key == ' ') {
        for (int i = 0; i < 30; ++i) {
            lines->add();
            points->add();
        }
    }

    if (key == 'l') {
        for (int i = 0; i < 10; ++i) {
            lines->add();
        }
    }

    if (key == 'p') {
        for (int i = 0; i < 100; ++i) {
            points->add();
        }
    }

    if (key == 'c') {
        lines->clear();
        points->clear();
    }

    if (key == 'b') {
        rasterizer->toggleBoundingBoxes();
    }

    if (key == 'o') {
        if (projectionMode == PERSPECTIVE) {
            projectionMode = ORTHO;
        } else {
            projectionMode = PERSPECTIVE;
        }
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

    if (button == GLUT_MOUSEWHEEL_DOWN && state == GLUT_DOWN) {
        camera->handleKeyPress('a');
    }

    if (button == GLUT_MOUSEWHEEL_UP && state == GLUT_DOWN) {
        camera->handleKeyPress('z');
    }
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowSize(width, height);
    glutCreateWindow("srender");

    glutIdleFunc(idle);
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutMotionFunc(motion);
    glutMouseFunc(mouse);

    rasterizer = std::make_unique<render::Rasterizer>();
    renderTarget.viewport = std::make_shared<render::Viewport>(64, 64, width-128, height-128);
    renderTarget.framebuffer = std::make_shared<render::Framebuffer>(width, height);
    renderTarget.depthbuffer = std::make_shared<render::Depthbuffer>(width, height);

    shaders.vertexShader = std::make_shared<render::DefaultVertexTransform>();
    shaders.fragmentShader = std::make_shared<render::InputColorShader>();

    rasterizer->setRenderOutput(renderTarget);
    rasterizer->setShaders(shaders);

    lines = std::make_unique<RandomLinesGeometry>(glm::vec3(-10), glm::vec3(10));
    points = std::make_unique<RandomPointsGeometry>(glm::vec3(-10), glm::vec3(10));
    camera = std::make_unique<OrbitCamera>(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 15.0f);

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
