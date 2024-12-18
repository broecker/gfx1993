#include "DemoApp.h"

#include <cstdlib>
#include <iostream>
#include <iomanip>

#include <glm/ext.hpp>

namespace {

static const int GLUT_MOUSEWHEEL_DOWN = 3;
static const int GLUT_MOUSEWHEEL_UP = 4;

static const int WIDTH_VGA = 640;
static const int HEIGHT_VGA = 480;

}  // namespace

DemoApp *DemoApp::appInstance = nullptr;
DemoApp::DemoApp(const std::string &name)
    : name(name), width(WIDTH_VGA), height(HEIGHT_VGA),
      logFrameTime(true), mousePosition(0,0) {
  rasterizer = std::make_unique<render::Rasterizer>();

  renderConfig.viewport =
      std::make_shared<render::Viewport>(0, 0, width, height);
  renderConfig.framebuffer =
      std::make_shared<render::Framebuffer>(width, height);
  renderConfig.depthbuffer =
      std::make_shared<render::Depthbuffer>(width, height);

  camera = std::make_unique<OrbitCamera>(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0),
                                         30.0f);

  srand(time(0));
}

void DemoApp::run(int argc, char **argv) {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
		return;
	}

	window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN );
	if (window == nullptr) {
		std::cerr << "Error creating SDL window: " << SDL_GetError() << std::endl;
		return;
	}

  // Ugly hack :-/
  appInstance = this;

  this->init();

  running = true;

  const int64_t startTicks = SDL_GetTicks64();
	int64_t lastSecond = startTicks;

  while (running) {
    // Performance measurements.
		frames++;
		totalFrames++;
		const int64_t nowTicks = SDL_GetTicks64();
		const float dt = static_cast<float>(nowTicks) / lastSecond / 1000.0;

    appInstance->updateFrame(dt);
    appInstance->renderFrame();

    handleEvents();

    blitSurface();
    SDL_UpdateWindowSurface(window);

		if ((nowTicks - lastSecond) > 1000) {
      if (appInstance->logFrameTime) {
			  std::cout << "FPS: " << std::setw(3) << frames << " [avg: " << std::setprecision(4) << static_cast<float>(totalFrames) / (nowTicks - startTicks) * 1000  << "\ttotal frames: " << std::setw(5) << totalFrames << ", time: " << std::setprecision(5) <<  static_cast<float>(nowTicks - startTicks) / 1000 << "s]\tdt: " << std::setprecision(5) << dt << std::endl;
      }
			frames = 0;
			lastSecond = nowTicks;
		}
  }  

  SDL_DestroyWindow(window);
	SDL_Quit();
}

void DemoApp::blitSurface() {
  SDL_Surface* surface = SDL_GetWindowSurface(window);

  SDL_LockSurface(surface);

  // This is messy -- ideally we should already write as uint8 in the last step in the rasterizer.
  std::vector<uint8_t> pixels = renderConfig.framebuffer->getUint8RgbaBuffer();

  // Switch to BGRA
  for (size_t i = 0; i < pixels.size(); i += 4) {
    std::swap(pixels[i+0], pixels[i+2]);
  }
  memcpy(surface->pixels, &pixels[0], pixels.size());

  // Does not work.
  // SDL_ConvertPixels(renderConfig.framebuffer->getWidth(),
  //                   renderConfig.framebuffer->getHeight(),
  //                   SDL_PIXELFORMAT_RGBA32,
  //                   &pixels[0],
  //                   renderConfig.framebuffer->getWidth(),
  //                   surface->format->format,
  //                   surface->pixels,
  //                   surface->pitch);

  SDL_UnlockSurface(surface);
}

void DemoApp::handleEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      running = false;
    }

    if (event.type == SDL_KEYDOWN) {
      switch (event.key.keysym.sym) {
      case SDLK_q:
      case SDLK_ESCAPE:
        running = false;
        break;
      default:
        break;
      }

      appInstance->handleKeyboard(event.key.keysym.sym, mousePosition);
    }

    if (event.type == SDL_MOUSEMOTION) {
      appInstance->handleMotion(glm::ivec2(event.motion.x, event.motion.y));
    }

    if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
      appInstance->handleMouse(event.button.button, event.button.state, glm::ivec2(event.button.x, event.button.y));
    }
  }
}

void DemoApp::handleKeyboard(unsigned char key, const glm::ivec2& mousePosition) {}

void DemoApp::handleMouse(int button, int state, const glm::ivec2& mousePosition) {
  this->mousePosition = mousePosition;

  if (button == GLUT_MOUSEWHEEL_DOWN && state == 1) {
    camera->handleKeyPress('a');
  }

  if (button == GLUT_MOUSEWHEEL_UP && state == 0) {
    camera->handleKeyPress('z');
  }
}

void DemoApp::handleMotion(const glm::ivec2& newMousePosition) {
  const glm::ivec2 delta = newMousePosition - mousePosition;
  this->mousePosition = newMousePosition;
  camera->handleMouseMove(delta);
}

void DemoApp::updateFrame(float dt)
{
  rasterizer->resetDebugInfo();
}
