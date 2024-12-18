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
  std::string name;
  int width, height;

  bool running;
  bool logFrameTime;
  
  int frames = 0;
	int totalFrames = 0;

  render::RenderConfig renderConfig;
  std::unique_ptr<render::Rasterizer> rasterizer;

  std::unique_ptr<Camera> camera;

  SDL_Window* window = nullptr;

  glm::ivec2 mousePosition;

  virtual void init() = 0;

  virtual void updateFrame(float dt);

  virtual void renderFrame() = 0;

  virtual void handleKeyboard(unsigned char key, const glm::ivec2& mousePosition);

  virtual void handleMouse(int button, int state, const glm::ivec2& mousePosition);

  virtual void handleMotion(const glm::ivec2& mousePosition);

  void blitSurface();

private:
  static DemoApp *appInstance;

  void handleEvents();
};

#endif // GFX1993_GLUTDEMOAPP_H
