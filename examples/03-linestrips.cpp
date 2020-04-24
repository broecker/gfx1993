#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/polar_coordinates.hpp>

#include <GL/glut.h>

#include "rendering/Framebuffer.h"
#include "rendering/Depthbuffer.h"
#include "rendering/Pipeline.h"
#include "rendering/Shader.h"
#include "rendering/Rasterizer.h"
#include "rendering/Viewport.h"
#include "common/Camera.h"
#include "geometry/GridGeometry.h"
#include "geometry/PlyGeometry.h"

// OpenGL texture Id
unsigned int texture;
unsigned int width = 640, height = 480;

class WireSphere : public geometry::Geometry
{
public:
    WireSphere(float radius)
    {
        // Circles of latitude (N - S), excluding the poles.
        for (int theta = -80; theta < 90; theta += 10) {
            // A full circle on the horizontal plane.
            for (int phi = 0; phi <= 360; phi += 10) {
                glm::vec3 pos = glm::euclidean(glm::radians(glm::vec2(theta, phi))) * radius;
                glm::vec3 normal = glm::normalize(pos);
                glm::vec2 texcoord = glm::vec2(phi, theta) / glm::vec2(90, 360);
                glm::vec4 color = glm::vec4(1);

                render::Vertex v(glm::vec4(pos, 1), normal, color, texcoord);
                vertices.push_back(v);
                indices.push_back(vertices.size() - 1);
            }
        }

        // Circles of longitude (E-W).
        for (int phi = 0; phi <= 360; phi += 10) {
            // A full circle on the horizontal plane.
            for (int theta = -80; theta < 90; theta += 10) {
                glm::vec3 pos = glm::euclidean(glm::radians(glm::vec2(theta, phi))) * radius;
                glm::vec3 normal = glm::normalize(pos);
                glm::vec2 texcoord = glm::vec2(phi, theta) / glm::vec2(90, 360);
                glm::vec4 color = glm::vec4(1);

                render::Vertex v(glm::vec4(pos, 1), normal, color, texcoord);
                vertices.push_back(v);
                indices.push_back(vertices.size() - 1);
            }
        }

        // Note, this creates a giant 'tunnel' through the sphere's center. Ideally, we want to draw all the line
        // loops in multiple draw calls.
    }
};

std::unique_ptr<WireSphere> sphere;

std::unique_ptr<render::Rasterizer> rasterizer;
render::Rasterizer::ShaderConfiguration shaders;
render::Rasterizer::RenderOutput renderTarget;

std::unique_ptr<Camera> camera;

glm::ivec2 mousePosition;

static const int GLUT_MOUSEWHEEL_DOWN = 3;
static const int GLUT_MOUSEWHEEL_UP = 4;

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
    dvt->projectionMatrix = camera->getProjectionMatrix();

    try {
        rasterizer->drawLineStrip(sphere->getVertices(), sphere->getIndices());
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

    if (key == 'b') {
        rasterizer->toggleBoundingBoxes();
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

    srand(time(0));

    rasterizer = std::make_unique<render::Rasterizer>();
    renderTarget.viewport = std::make_shared<render::Viewport>(0, 0, width, height);
    renderTarget.framebuffer = std::make_shared<render::Framebuffer>(width, height);
    renderTarget.depthbuffer = std::make_shared<render::Depthbuffer>(width, height);

    shaders.vertexShader = std::make_shared<render::DefaultVertexTransform>();
    shaders.fragmentShader = std::make_shared<render::SingleColorShader>(glm::vec4(0, 0, 1, 1));;

    rasterizer->setRenderOutput(renderTarget);
    rasterizer->setShaders(shaders);

    sphere = std::make_unique<WireSphere>(10.0f);
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
