#include <cstdlib>
#include <iostream>
#include <memory>

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <SDL.h>

#include "rendering/Framebuffer.h"
#include "rendering/Depthbuffer.h"
#include "rendering/Shader.h"
#include "rendering/Rasterizer.h"
#include "rendering/Viewport.h"
#include "common/Camera.h"
#include "geometry/CubeGeometry.h"
#include "geometry/RandomTriangleGeometry.h"
#include "geometry/GridGeometry.h"
#include "geometry/PlyGeometry.h"

constexpr unsigned int width = 640, height = 480;

std::unique_ptr<geometry::CubeGeometry> cube = nullptr;
std::unique_ptr<geometry::RandomTriangleGeometry> triangles = nullptr;
std::unique_ptr<geometry::GridGeometry> grid;

std::vector<std::unique_ptr<geometry::PlyGeometry> > bunnyList;

render::Rasterizer::RenderOutput renderTarget;
render::Rasterizer::ShaderConfiguration shaders;

std::unique_ptr<render::Rasterizer> rasterizer;
std::shared_ptr<render::DefaultVertexTransform> vertexTransform;
std::shared_ptr<render::FragmentShader> singleColorShader;
std::shared_ptr<render::FragmentShader> normalShader;

Camera *camera;

glm::ivec2 mousePosition;

static const int GLUT_MOUSEWHEEL_DOWN = 3;
static const int GLUT_MOUSEWHEEL_UP = 4;

static void render(SDL_Surface* surface)
{
    // Surface is BGRA and this is a dark blue.
    surface->clear(Color{50,0,0});

    SDL_LockSurface(surface);
    
    // Copy data here ... from float to BGRA uint8.
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, renderTarget.framebuffer->getPixels());

    SDL_UnlockSurface(surface);
    
}

static void update(float dt)
{
    // clear the buffers
    renderTarget.framebuffer->clear(glm::vec4(0, 0, 0.2f, 0));
    renderTarget.depthbuffer->clear();

    // reset the render matrices
    vertexTransform->modelMatrix = glm::mat4(1.f);
    vertexTransform->viewMatrix = camera->getViewMatrix();
    vertexTransform->projectionMatrix = camera->getProjectionMatrix();

    try {
        if (triangles) {
            rasterizer->drawTriangles(triangles->getVertices(), triangles->getIndices());
        }

        if (grid) {
            shaders.fragmentShader = singleColorShader;
            rasterizer->setShaders(shaders);
            rasterizer->drawLines(grid->getVertices(), grid->getIndices());
        }

        if (cube) {
            shaders.fragmentShader = singleColorShader;
            rasterizer->setShaders(shaders);
            rasterizer->drawLines(cube->getVertices(), cube->getIndices());
        }

        if (!bunnyList.empty()) {
            shaders.fragmentShader = normalShader;
            rasterizer->setShaders(shaders);
            for (auto bunny = bunnyList.begin(); bunny != bunnyList.end(); ++bunny) {
                vertexTransform->modelMatrix = (*bunny)->transform;
                rasterizer->drawTriangles((*bunny)->getVertices(), (*bunny)->getIndices());
            }
        }
    }
    catch (const char *txt) {
        std::cerr << "Render error :\"" << txt << "\"\n";
    }
}

static void keyboard(unsigned char key, int x, int y)
{
    if (key == 27)
        exit(0);

    if (key == 't') {
        if (!triangles)
            triangles = std::make_unique<geometry::RandomTriangleGeometry>(glm::vec3(-10, -10, -10), glm::vec3(10, 10, 10));
        triangles->addTriangle();
    }

    if (key == 'b') {
        rasterizer->toggleBoundingBoxes();
    }

    if (key == 'c') {
        // create a cube
        cube = std::make_unique<geometry::CubeGeometry>(glm::vec3(2.f, 2.f, 2.f));
    }

    if (key == 'g') {
        std::unique_ptr<geometry::PlyGeometry> bunny = std::make_unique<geometry::PlyGeometry>();
        bunny->loadPly("models/bunny/reconstruction/bun_zipper_res3.ply");

        float randomAngle = (float) rand() / RAND_MAX;
        glm::vec3 randomAxis = glm::sphericalRand(1);

        const glm::vec4 minBounds(-15, -4, -15, 1);
        const glm::vec4 maxBounds(15, 15, 15, 1);
        const glm::vec3 minScale(15, 15, 15);
        const glm::vec3 maxScale(30, 30, 30);

        bunny->transform = glm::rotate(randomAngle, randomAxis);
        bunny->transform[3] = glm::linearRand(minBounds, maxBounds);
        bunny->transform *= glm::scale(glm::linearRand(minScale, maxScale));

        bunnyList.emplace_back(std::move(bunny));
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

    if (button == GLUT_MOUSEWHEEL_DOWN && state == GLUT_DOWN) {
        camera->handleKeyPress('a');

    }

    if (button == GLUT_MOUSEWHEEL_UP && state == GLUT_DOWN) {
        camera->handleKeyPress('z');

    }
}

static int loop(SDL_Window* window) {
	bool running = true;
	int frames = 0;
	int total_frames = 0;

	const int64_t start_ticks = SDL_GetTicks64();
	int64_t last_second = start_ticks;

	boomer::Game game(SCREEN_WIDTH, SCREEN_HEIGHT);

	while (running) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			}

            if (event.type == SDL)

			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym){
				case SDLK_q:
				case SDLK_ESCAPE:
					running = false;
					break;
				default:
					break;
				}
			}
		}

		// Performance measurements.
		frames++;
		total_frames++;
		const int64_t now_ticks = SDL_GetTicks64();
		const float dt = static_cast<float>(now_ticks / last_second) / 1000.0;

		update(dt);
		render(SDL_GetWindowSurface(window));
		SDL_UpdateWindowSurface(window);

		if ((now_ticks - last_second) > 1000) {
			std::cout << "FPS: " << frames <<  " [avg: " << static_cast<float>(total_frames) / (now_ticks - start_ticks) * 1000  << "   total: " << total_frames << ", time: " << static_cast<float>(now_ticks - start_ticks) << "s] dt: " << dt << std::endl;
			frames = 0;
			last_second = now_ticks;
		}

	}

    return 0;
}

int main(int argc, char **argv)
{
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
		return -1;
	}

	SDL_Window* window = SDL_CreateWindow("Hello Boomer!", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
	if (window == nullptr) {
		std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
		return -2;
	}

    srand(time(0));

    rasterizer = std::make_unique<render::Rasterizer>();
    renderTarget.viewport = std::make_shared<render::Viewport>(0, 0, width, height);
    renderTarget.framebuffer = std::make_shared<render::Framebuffer>(width, height);
    renderTarget.depthbuffer = std::make_shared<render::Depthbuffer>(width, height);
    rasterizer->setRenderOutput(renderTarget);

    vertexTransform = std::make_shared<render::DefaultVertexTransform>();
    singleColorShader = std::make_shared<render::InputColorShader>();
    shaders.vertexShader = vertexTransform;
    shaders.fragmentShader = singleColorShader;
    normalShader = std::make_shared<render::NormalColorShader>();
    rasterizer->setShaders(shaders);

    grid = std::make_unique<geometry::GridGeometry>();
    camera = new OrbitCamera(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0), 10.0f);

    return loop();
}
