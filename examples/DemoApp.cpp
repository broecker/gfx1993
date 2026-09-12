#include "DemoApp.h"

#include "config.h"
#include "Profiler.h"

#include <cstdlib>
#include <iostream>
#include <iomanip>

#include <glm/ext.hpp>

#if GFX1993_DEMO_USE_OPENMP
  #include <omp.h>
#endif

using namespace gfx1993;

DemoApp *DemoApp::appInstance = nullptr;
DemoApp::DemoApp(const std::string &name)
    : name(name), width(gfx1993::VGA_WIDTH), height(gfx1993::VGA_HEIGHT),
      logFrameTime(true), mousePosition(0,0) {
  rasterizer = std::make_unique<Rasterizer>();

  renderConfig.viewport =
      std::make_shared<Viewport>(0, 0, width, height);
  renderConfig.framebuffer =
      std::make_shared<Framebuffer>(width, height);
  renderConfig.depthbuffer =
      std::make_shared<Depthbuffer>(width, height);

  camera = std::make_unique<OrbitCamera>(glm::vec3(0, 0, 0), 30.0f);

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
	int64_t lastTicks = startTicks;
  int64_t lastSecond = startTicks;

  while (running) {
    // Performance measurements.
		frames++;
		totalFrames++;
		const int64_t nowTicks = SDL_GetTicks64();

    const int64_t ticks = nowTicks - lastTicks;
    float dt = static_cast<float>(ticks) / 1000.f;
    lastTicks = nowTicks;

    handleEvents();

    appInstance->updateFrame(dt);
    appInstance->renderFrame();

    blitSurface();
    SDL_UpdateWindowSurface(window);
    GFX1993_FRAME_MARK();

		if ((nowTicks - lastSecond) > 1000) {
      if (appInstance->logFrameTime) {
			  std::cout << "FPS: " << std::setw(3) << frames << " [avg: " << std::setprecision(4) << static_cast<float>(totalFrames) / (nowTicks - startTicks) * 1000  << "\ttotal frames: " << std::setw(5) << totalFrames << ", time: " << std::setprecision(5) <<  static_cast<float>(nowTicks - startTicks) / 1000 << "s]\tdt: " << std::setprecision(5) << dt << std::endl;
        rasterizer->getDebugInfo().print();
        rasterizer->resetDebugInfo();
      }
			frames = 0;
			lastSecond = nowTicks;
		}

    SDL_Delay(0);
  }

  SDL_DestroyWindow(window);
	SDL_Quit();
}

void DemoApp::blitSurface() {
  GFX1993_ZONE_N("app.blit");

  SDL_Surface* surface = SDL_GetWindowSurface(window);
  SDL_LockSurface(surface);

  // Resample buffer and switch to BGRA format.
  {
    GFX1993_ZONE_N("app.blit.resample");
    #pragma omp parallel for
    for (unsigned int h = 0; h < height; ++h) {
      for (unsigned int w = 0; w < width; ++w) {
        glm::vec2 coord(static_cast<float>(w) / width,
                        static_cast<float>(h) / height);

        const glm::vec4& pixel = renderConfig.framebuffer->getPixel(coord);
        // Also flip the y-axis.
        SDL_Color& c = pixels[w + h*width];

        // Also switch to BGRA.
        c.r = static_cast<Uint8>(pixel.b * 255);
        c.g = static_cast<Uint8>(pixel.g * 255);
        c.b = static_cast<Uint8>(pixel.r * 255);
        c.a = static_cast<Uint8>(pixel.a * 255);
      }
    }
  }

  {
    GFX1993_ZONE_N("app.blit.copy");
    memcpy(surface->pixels, &pixels[0], pixels.size());
  }

  SDL_UnlockSurface(surface);
}

void DemoApp::handleEvents() {
  GFX1993_ZONE_N("app.events");

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      running = false;
    }
    handleEvent(event);
  }
}

void DemoApp::handleEvent(const SDL_Event& event) {
 if (event.type == SDL_KEYDOWN) {
  switch (event.key.keysym.sym) {
    case SDLK_ESCAPE:
      running = false;
      break;
    default:
      break;
    }

    appInstance->handleKeyboard(event.key.keysym.sym, mousePosition);
  }

  // TODO: handle keyup.
  if (event.type == SDL_MOUSEMOTION) {
    appInstance->handleMotion(glm::ivec2(event.motion.x, event.motion.y));
  }

  if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_MOUSEBUTTONUP) {
    appInstance->handleMouse(event.button.button, event.button.state, glm::ivec2(event.button.x, event.button.y));
  }

  if (event.type == SDL_MOUSEWHEEL) {
    appInstance->handleMouseWheel(event.wheel.y, mousePosition);
  }

  if (event.type == SDL_WINDOWEVENT) {
    if (event.window.event ==  SDL_WINDOWEVENT_RESIZED) {
      handleResize(event.window.data1, event.window.data2);
      std::cout << "Resized window to " << event.window.data1 << "x" << event.window.data2 << std::endl;
    }
  }
}

void DemoApp::handleKeyboard(unsigned char key, const glm::ivec2& mousePosition) {
  if (key == '-') {
    camera->handleInputTranslate(glm::vec3(0,0,1));
  }
  if (key == '+' || key == '=') {
    camera->handleInputTranslate(glm::vec3(0,0,-1));
  }
}

void DemoApp::handleMouse(int button, int state, const glm::ivec2& mousePosition) {
  this->mousePosition = mousePosition;
}

void DemoApp::handleMotion(const glm::ivec2& newMousePosition) {
  const glm::ivec2 delta = newMousePosition - mousePosition;
  this->mousePosition = newMousePosition;
  
  camera->handleInputRotate(glm::vec3(delta.y, delta.x, 0.f));
}

void DemoApp::handleMouseWheel(int wheel, const glm::ivec2& mousePosition) {
  // For an orbit camera, translation along Z means adjusting the radius.
  camera->handleInputTranslate(glm::vec3(0,0, wheel));
}

void DemoApp::updateFrame(float dt) {
  camera->update(dt);
}

void DemoApp::handleResize(unsigned int w, unsigned int h) {
  pixels.resize(w*h*4);
  width = w;
  height = h;
}
