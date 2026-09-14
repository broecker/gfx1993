#ifndef GFX1993_GLUTDEMOAPP_H
#define GFX1993_GLUTDEMOAPP_H

#include <memory>
#include <string>

#include <SDL2/SDL.h>

#include "Pipeline.h"
#include "Camera.h"
#include "Depthbuffer.h"
#include "Framebuffer.h"
#include "RenderConfig.h"
#include "Rasterizer.h"
#include "Shader.h"
#include "Viewport.h"

class DemoApp {
public:
  DemoApp(const std::string &name);
  virtual ~DemoApp() = default;

  virtual void run(int argc, char **argv);

protected:
  std::string name;
  unsigned int width, height;

  bool running;
  bool showStatsOverlay;

  int frames = 0;
	int totalFrames = 0;

  gfx1993::RenderConfig renderConfig;
  std::unique_ptr<gfx1993::Rasterizer> rasterizer;

  std::unique_ptr<gfx1993::Camera> camera;

  glm::ivec2 mousePosition;

  virtual void init() = 0;

  virtual void updateFrame(float dt);

  virtual void renderFrame() = 0;

  virtual void handleEvent(const SDL_Event& e);


  // Convenience methods, called by handleEvent above;
  virtual void handleKeyboard(unsigned char key, const glm::ivec2& mousePosition);

  virtual void handleMouse(int button, int state, const glm::ivec2& mousePosition);

  virtual void handleMotion(const glm::ivec2& mousePosition);

  virtual void handleMouseWheel(int wheel, const glm::ivec2& mousePosition);

private:
  static DemoApp *appInstance;

  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;
  // Streaming texture the software-rendered framebuffer is uploaded into
  // every frame; the SDL renderer then composites it with the ImGui overlay.
  SDL_Texture* frameTexture = nullptr;

  // This is the raw pixel buffer we will display. It has been converted
  // from the interal float rgba format.
  std::vector<SDL_Color> pixels;

  // Perf stats, refreshed once a second and shown in the stats overlay.
  float currentFPS = 0.f;
  float avgFPS = 0.f;
  float frameTimeMs = 0.f;

  void updateFrameTexture();

  void drawStatsOverlay();

  void handleEvents();

  void handleResize(unsigned int width, unsigned int height);

  void createFrameTexture();
};

#endif // GFX1993_GLUTDEMOAPP_H
