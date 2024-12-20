#ifndef GFX1993_GLUTDEMOAPP_H
#define GFX1993_GLUTDEMOAPP_H

#include <memory>
#include <string>

#include <SDL2/SDL.h>

#include "common/Camera.h"
#include "rendering/Depthbuffer.h"
#include "rendering/Framebuffer.h"
#include "rendering/Pipeline.h"
#include "rendering/Rasterizer.h"
#include "rendering/RenderConfig.h"
#include "rendering/Shader.h"
#include "rendering/Viewport.h"

class DemoApp {
public:
  DemoApp(const std::string &name);
  virtual ~DemoApp() = default;

  virtual void run(int argc, char **argv);

protected:
  int width, height;

  bool running;
  bool logFrameTime;
  
  int frames = 0;
	int totalFrames = 0;

  gfx1993::render::RenderConfig renderConfig;
  std::unique_ptr<gfx1993::render::Rasterizer> rasterizer;

  std::unique_ptr<gfx1993::common::Camera> camera;

  glm::ivec2 mousePosition;

  virtual void init() = 0;

  virtual void updateFrame(float dt);

  virtual void renderFrame() = 0;

  virtual void handleKeyboard(unsigned char key, const glm::ivec2& mousePosition);

  virtual void handleMouse(int button, int state, const glm::ivec2& mousePosition);

  virtual void handleMotion(const glm::ivec2& mousePosition);

private:
  static DemoApp *appInstance;

  std::string name;

  SDL_Window* window = nullptr;
  // This is the raw pixel buffer we will display. It has been converted
  // from the interal float rgba format.
  std::vector<SDL_Color> pixels;

  void blitSurface();

  void handleEvents();

  void handleResize(unsigned int width, unsigned int height);
};

#endif // GFX1993_GLUTDEMOAPP_H
