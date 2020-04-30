#ifndef GFX1993_GLUTDEMOAPP_H
#define GFX1993_GLUTDEMOAPP_H

#include <memory>

#include "common/Camera.h"
#include "rendering/Depthbuffer.h"
#include "rendering/Framebuffer.h"
#include "rendering/Pipeline.h"
#include "rendering/Rasterizer.h"
#include "rendering/RenderConfig.h"
#include "rendering/Shader.h"
#include "rendering/Viewport.h"

class GlutDemoApp {
public:
  GlutDemoApp(const std::string &name);
  virtual ~GlutDemoApp();

  void run(int argc, char **argv);

protected:
  std::string name;
  int width, height;
  unsigned int texture;

  bool logFrameTime;

  render::RenderConfig renderConfig;
  std::unique_ptr<render::Rasterizer> rasterizer;

  std::unique_ptr<Camera> camera;

  glm::ivec2 mousePosition;

  virtual void init() = 0;

  virtual void updateFrame(float dt);

  virtual void renderFrame() = 0;

  virtual void handleKeyboard(unsigned char key, int x, int y);

  virtual void handleMouse(int button, int state, int x, int y);

  virtual void handleMotion(int x, int y);

private:
  static GlutDemoApp *appInstance;

  static void glutDisplay();
  static void glutIdle();
  static void glutKeyboard(unsigned char key, int x, int y);
  static void glutMotion(int x, int y);
  static void glutMouse(int button, int state, int x, int y);
};

#endif // GFX1993_GLUTDEMOAPP_H
