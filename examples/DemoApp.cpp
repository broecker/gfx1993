#include "DemoApp.h"

#include <cstdlib>
#include <iostream>
#include <iomanip>

#include <glm/ext.hpp>

namespace {

static const int WIDTH_VGA = 320;
static const int HEIGHT_VGA = 240;

}  // namespace

using namespace gfx1993;
using namespace render;
using namespace common;

DemoApp *DemoApp::appInstance = nullptr;
DemoApp::DemoApp(const std::string &name)
    : name(name), width(WIDTH_VGA), height(HEIGHT_VGA),
      logFrameTime(true), mousePosition(0,0) {
  rasterizer = std::make_unique<Rasterizer>();

  renderConfig.viewport =
      std::make_shared<Viewport>(0, 0, width, height);
  renderConfig.framebuffer =
      std::make_shared<Framebuffer>(width, height);
  renderConfig.depthbuffer =
      std::make_shared<Depthbuffer>(width, height);

  camera = std::make_unique<OrbitCamera>(glm::vec3(0, 0, 0), glm::vec3(0, 1, 0),
                                         30.0f);

  srand(time(0));
  handleResize(width, height);
}

void DemoApp::run(int argc, char **argv) {
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
		std::cerr << "Unable to initialize SDL: " << SDL_GetError() << std::endl;
		return;
	}

	window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE );
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
        rasterizer->getProfile().print();
        rasterizer->getDebugInfo().print();
        rasterizer->resetDebugInfo();
      }
			frames = 0;
			lastSecond = nowTicks;
		}
  }  

  SDL_DestroyWindow(window);
	SDL_Quit();
}

void DemoApp::blitSurface() {
  auto blitProf = rasterizer->getProfile().startTiming("app.blit");

  SDL_Surface* surface = SDL_GetWindowSurface(window);
  SDL_LockSurface(surface);

  // Resample buffer and switch to BGRA format.
  {
    const auto resampleProf = rasterizer->getProfile().startTiming("app.blit.resample");
    #pragma omp parallel for
    for (unsigned int w = 0; w < width; ++w) {
      for (unsigned int h = 0; h < height; ++h) {
        glm::vec2 coord(static_cast<float>(w) / width,
                        static_cast<float>(h) / height);

        const glm::vec4& pixel = renderConfig.framebuffer->getPixel(coord);
        // Also flip the y-axis.
        SDL_Color& c = pixels[w + (height-h)*width];

        // Also switch to BGRA.
        c.r = static_cast<Uint8>(pixel.b * 255);
        c.g = static_cast<Uint8>(pixel.g * 255);
        c.b = static_cast<Uint8>(pixel.r * 255);
        c.a = static_cast<Uint8>(pixel.a * 255);
      }
    }
  }

  {
    auto copyProf = rasterizer->getProfile().startTiming("app.blit.copy");
    //std::cout << "Src: " << renderConfig.framebuffer->getWidth() << "x" << renderConfig.framebuffer->getHeight() << "; dest: " << surface->w << "x" << surface->h << std::endl;
    memcpy(surface->pixels, &pixels[0], pixels.size());
  }

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
  auto eventProf = rasterizer->getProfile().startTiming("app.events");

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

    if (event.type == SDL_MOUSEWHEEL) {
      if (event.wheel.y > 0) {
        camera->handleKeyPress('a');
      }
      if (event.wheel.y < 0) {
        camera->handleKeyPress('z');
      }
    }

    if (event.type == SDL_WINDOWEVENT) {
      if (event.window.event ==  SDL_WINDOWEVENT_RESIZED) {
        handleResize(event.window.data1, event.window.data2);
        std::cout << "Resized window to " << event.window.data1 << "x" << event.window.data2 << std::endl;
      }
    }
  }

  rasterizer->getProfile().endTiming(eventProf);
}

void DemoApp::handleKeyboard(unsigned char key, const glm::ivec2& mousePosition) {}

void DemoApp::handleMouse(int button, int state, const glm::ivec2& mousePosition) {
  this->mousePosition = mousePosition;
}

void DemoApp::handleMotion(const glm::ivec2& newMousePosition) {
  const glm::ivec2 delta = newMousePosition - mousePosition;
  this->mousePosition = newMousePosition;
  camera->handleMouseMove(delta);
}

void DemoApp::updateFrame(float dt) {
}

void DemoApp::handleResize(unsigned int w, unsigned int h) {
  pixels.resize(w*h*4);
  width = w;
  height = h;
}
