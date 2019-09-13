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

class Quad : public geo::Geometry
{
public:
    // Constructs a quad from 4 vertices. Assumes the following geometry:
    // a +---+ d
    //   | \ |
    // b +---+ c
    // The size will be [-1..1] along the XY axis.
    Quad(const glm::vec4 &color)
    {
        using render::Vertex;
        using glm::vec2;
        using glm::vec4;

        const glm::vec3 normal(0, 0, 1);

        Vertex va = Vertex(vec4(-1, 1, 0, 1), normal, color, vec2(0, 1));
        Vertex vb = Vertex(vec4(-1, -1, 0, 1), normal, color, vec2(0, 0));
        Vertex vc = Vertex(vec4(1, -1, 0, 1), normal, color, vec2(1, 0));
        Vertex vd = Vertex(vec4(1, 1, 0, 1), normal, color, vec2(1, 1));

        vertices.push_back(va);
        vertices.push_back(vb);
        vertices.push_back(vc);
        vertices.push_back(vd);

        indices.push_back(0);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(2);
        indices.push_back(3);
        indices.push_back(0);

        // And back faces (although normal is wrong ... )
        indices.push_back(0);
        indices.push_back(2);
        indices.push_back(1);
        indices.push_back(2);
        indices.push_back(0);
        indices.push_back(3);
    }
};

std::vector<std::unique_ptr<Quad> > quads;

std::unique_ptr<render::Rasterizer> rasterizer;
render::Rasterizer::ShaderConfiguration shaders;
render::Rasterizer::RenderOutput renderTarget;

std::unique_ptr<Camera> camera;

glm::ivec2 mousePosition;

static const int GLUT_MOUSEWHEEL_DOWN = 3;
static const int GLUT_MOUSEWHEEL_UP = 4;

bool sortByDepth = false;

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


    if (sortByDepth) {
        const glm::mat4 &viewMatrix = camera->getViewMatrix();
        std::sort(quads.begin(), quads.end(),
                  [&viewMatrix](const std::unique_ptr<Quad> &a, const std::unique_ptr<Quad> &b) {
                      // Calculates the quad's position in eye coordinates.
                      const glm::vec4 posA = viewMatrix * a->transform * glm::vec4(0, 0, 0, 1);
                      const glm::vec4 posB = viewMatrix * b->transform * glm::vec4(0, 0, 0, 1);

                      // Compares the distance from the eye.
                      return posA.z < posB.z;
                  });
    }

    try {
        for (auto quad = quads.begin(); quad != quads.end(); ++quad) {
            dvt->modelMatrix = quad->get()->transform;
            rasterizer->drawTriangles(quad->get()->getVertices(), quad->get()->getIndices());
        }
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

    if (key == 'd') {
        sortByDepth = !sortByDepth;
        std::cout << (sortByDepth ? "S" : "Not s") << "orting by depth." << std::endl;
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

    rasterizer = std::make_unique<render::Rasterizer>();
    renderTarget.viewport = std::make_shared<render::Viewport>(0, 0, width, height);
    renderTarget.framebuffer = std::make_shared<render::Framebuffer>(width, height);
    renderTarget.depthbuffer = std::make_shared<render::Depthbuffer>(width, height);

    shaders.vertexShader = std::make_shared<render::DefaultVertexTransform>();
    shaders.fragmentShader = std::make_shared<render::InputColorShader>();

    rasterizer->setRenderOutput(renderTarget);
    rasterizer->setShaders(shaders);

    // Create 3 slightly offset quads.
    std::unique_ptr<Quad> q = std::make_unique<Quad>(glm::vec4(0.8f, 0.2f, 0.f, 0.4f));
    q->transform = glm::translate(glm::vec3(-2, 1, 2));
    quads.emplace_back(std::move(q));

    q = std::make_unique<Quad>(glm::vec4(0.1f, 0.9f, 0.f, 0.4f));
    q->transform = glm::translate(glm::vec3(0, 1, 0));
    quads.emplace_back(std::move(q));

    q = std::make_unique<Quad>(glm::vec4(0.1f, 0.1f, 0.8f, 0.4f));
    q->transform = glm::translate(glm::vec3(2, 1, -2));
    quads.emplace_back(std::move(q));

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
